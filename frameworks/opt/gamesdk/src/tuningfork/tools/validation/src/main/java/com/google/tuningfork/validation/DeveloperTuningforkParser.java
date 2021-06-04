/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License
 */

package com.google.tuningfork.validation;

import static java.nio.charset.StandardCharsets.UTF_8;

import com.google.common.collect.ImmutableList;
import com.google.common.flogger.FluentLogger;
import com.google.common.io.Files;
import com.google.protobuf.ByteString;
import com.google.protobuf.Descriptors.Descriptor;
import com.google.protobuf.Descriptors.FileDescriptor;
import com.google.tuningfork.Tuningfork.Settings;
import java.io.File;
import java.io.IOException;
import java.util.Optional;
import java.util.regex.Pattern;

/** Tuningfork validation tool. Parses proto and settings files and validates them. */
public class DeveloperTuningforkParser {

  private Optional<File> devTuningforkProto = Optional.empty();
  private Optional<String> tuningforkSettings = Optional.empty();
  private ImmutableList<File> devFidelityFiles;

  private final ErrorCollector errors;
  private final ExternalProtoCompiler compiler;
  private final File folder;

  private static final FluentLogger logger = FluentLogger.forEnclosingClass();

  public DeveloperTuningforkParser(ErrorCollector errors, File folder, File protocBinary) {
    this.errors = errors;
    this.folder = folder;
    this.compiler = new ExternalProtoCompiler(protocBinary);
  }

  public void parseFilesInFolder() throws IOException {
    devTuningforkProto = findDevTuningforkProto(folder);
    tuningforkSettings = findTuningforkSettings(folder);
    devFidelityFiles = findDevFidelityParams(folder);
  }

  public void validate() throws IOException, CompilationException {
    if (devTuningforkProto.isPresent()) {
      logger.atInfo().log("File %s exists: OK", FolderConfig.DEV_TUNINGFORK_PROTO);
    } else {
      logger.atSevere().log("File %s exists: FAIL", FolderConfig.DEV_TUNINGFORK_PROTO);
    }

    File descriptorFile = new File(folder, FolderConfig.DEV_TUNINGFORK_DESCRIPTOR);

    FileDescriptor devTuningforkFileDesc =
        compiler.compile(devTuningforkProto.get(), Optional.of(descriptorFile));
    Descriptor annotationField = devTuningforkFileDesc.findMessageTypeByName("Annotation");
    Descriptor fidelityField = devTuningforkFileDesc.findMessageTypeByName("FidelityParams");

    ImmutableList<Integer> enumSizes =
        ValidationUtil.validateAnnotationAndGetEnumSizes(annotationField, errors);
    ValidationUtil.validateFidelityParams(fidelityField, errors);

    logger.atInfo().log("Loaded Annotation message:\n" + annotationField.toProto());
    logger.atInfo().log("Loaded FidelityParams message:\n" + fidelityField.toProto());

    validateAndSaveBinarySettings(enumSizes);
    encodeBinaryAndValidateDevFidelity(fidelityField);
  }

  private void validateAndSaveBinarySettings(ImmutableList<Integer> enumSizes) {
    if (tuningforkSettings.isPresent()) {
      logger.atInfo().log("File %s exists: OK", FolderConfig.TUNINGFORK_SETTINGS_TEXTPROTO);
    } else {
      logger.atSevere().log("File %s exists: FAIL", FolderConfig.TUNINGFORK_SETTINGS_TEXTPROTO);
    }

    if (errors.hasAnnotationErrors()) {
      logger.atSevere().log(
          "Skip %s file check as Annotation is not valid",
          FolderConfig.TUNINGFORK_SETTINGS_TEXTPROTO);
    }

    Optional<Settings> settings =
        ValidationUtil.validateSettings(enumSizes, tuningforkSettings.get(), errors);

    if (errors.hasSettingsErrors() || !settings.isPresent()) {
      logger.atSevere().log(
          "Skip saving %s file as %s contains errors",
          FolderConfig.TUNINGFORK_SETTINGS_BINARY, FolderConfig.TUNINGFORK_SETTINGS_TEXTPROTO);
    }

    logger.atInfo().log("Loaded settings:\n" + settings.get());

    saveBinarySettings(settings.get());
  }

  private void saveBinarySettings(Settings settings) {
    File outFile = new File(folder, FolderConfig.TUNINGFORK_SETTINGS_BINARY);
    try {
      Files.write(settings.toByteArray(), outFile);
    } catch (IOException e) {
      logger.atSevere().withCause(e).log(
          "Error writing settings to %s file", FolderConfig.TUNINGFORK_SETTINGS_BINARY);
    }
  }

  private void encodeBinaryAndValidateDevFidelity(Descriptor fidelityField) {
    if (!devFidelityFiles.isEmpty()) {
      logger.atInfo().log(
          "%d %s files found: OK\n %s",
          devFidelityFiles.size(),
          FolderConfig.DEV_FIDELITY_TEXTPROTO,
          devFidelityFiles.toString());
    } else {
      logger.atSevere().log("%s files found: FAIL", FolderConfig.DEV_FIDELITY_TEXTPROTO);
    }

    if (errors.hasFidelityParamsErrors()) {
      logger.atSevere().log(
          "Skip %s files check as FidelityParams message is not valid",
          FolderConfig.DEV_FIDELITY_TEXTPROTO);
    }

    devFidelityFiles.forEach(
        textprotoFile -> encodeBinaryAndValidateDevFidelity(fidelityField, textprotoFile));
  }

  private void encodeBinaryAndValidateDevFidelity(Descriptor fidelityField, File textprotoFile) {
    File binaryFile;
    try {
      binaryFile =
          compiler.encodeFromTextprotoFile(
              fidelityField.getFullName(),
              devTuningforkProto.get(),
              textprotoFile,
              getBinaryPathForTextprotoPath(textprotoFile),
              Optional.empty());
    } catch (IOException | CompilationException e) {
      errors.addError(
          ErrorType.DEV_FIDELITY_PARAMETERS_ENCODING,
          String.format("Encoding %s file", textprotoFile.getName()),
          e);
      return;
    }
    ByteString content;
    try {
      content = ByteString.copyFrom(Files.toByteArray(binaryFile));
    } catch (IOException e) {
      errors.addError(
          ErrorType.DEV_FIDELITY_PARAMETERS_READING,
          String.format("Reading %s file", binaryFile.getName()),
          e);
      return;
    }
    ValidationUtil.validateDevFidelityParams(content, fidelityField, errors);
  }

  private static String getBinaryPathForTextprotoPath(File textprotoFile) {
    return textprotoFile.getParentFile().getAbsolutePath()
        + "/"
        + textprotoFile.getName().replace(".txt", ".bin");
  }

  private static Optional<File> findDevTuningforkProto(File folder) throws IOException {
    File file = new File(folder, FolderConfig.DEV_TUNINGFORK_PROTO);
    return Optional.of(file);
  }

  private static Optional<String> findTuningforkSettings(File folder) throws IOException {
    File file = new File(folder, FolderConfig.TUNINGFORK_SETTINGS_TEXTPROTO);
    String content = Files.asCharSource(file, UTF_8).read();
    return Optional.of(content);
  }

  private static ImmutableList<File> findDevFidelityParams(File folder) throws IOException {
    Pattern devFidelityPattern = Pattern.compile(FolderConfig.DEV_FIDELITY_TEXTPROTO);
    return ImmutableList.copyOf(
        folder.listFiles((dir, filename) -> devFidelityPattern.matcher(filename).find()));
  }
}
