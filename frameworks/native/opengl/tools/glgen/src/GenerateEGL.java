/*
 * Copyright 2012 The Android Open Source Project
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
 * limitations under the License.
 */

import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.PrintStream;

public class GenerateEGL {

    private static void copy(String filename, PrintStream out) throws IOException {
        BufferedReader br = new BufferedReader(new FileReader(filename));
        String s;
        while ((s = br.readLine()) != null) {
            out.println(s);
        }
    }

    private static void emit(EGLCodeEmitter emitter,
                             BufferedReader specReader,
                             PrintStream glStream,
                             PrintStream cStream) throws Exception {
        String s = null;
        while ((s = specReader.readLine()) != null) {
            if (s.trim().startsWith("//")) {
                continue;
            }

            CFunc cfunc = CFunc.parseCFunc(s);

            String fname = cfunc.getName();
            String stubRoot = "stubs/egl/" + fname;
            String javaPath = stubRoot + ".java";
            File f = new File(javaPath);
            if (f.exists()) {
                System.out.println("Special-casing function " + fname);
                copy(javaPath, glStream);
                copy(stubRoot + ".cpp", cStream);

                // Register native function names
                // This should be improved to require fewer discrete files
                String filename = stubRoot + ".nativeReg";
                BufferedReader br =
                    new BufferedReader(new FileReader(filename));
                String nfunc;
                while ((nfunc = br.readLine()) != null) {
                    emitter.addNativeRegistration(nfunc);
                }
            } else {
                emitter.emitCode(cfunc, s);
            }
        }
    }

    public static void main(String[] args) throws Exception {
        int aidx = 0;
        while ((aidx < args.length) && (args[aidx].charAt(0) == '-')) {
            switch (args[aidx].charAt(1)) {
            default:
                System.err.println("Unknown flag: " + args[aidx]);
                System.exit(1);
            }

            aidx++;
        }

        BufferedReader checksReader =
            new BufferedReader(new FileReader("specs/egl/checks.spec"));
        ParameterChecker checker = new ParameterChecker(checksReader);


        for(String suffix: new String[] {"EGL14", "EGL15", "EGLExt"}) {
            BufferedReader specReader = new BufferedReader(new FileReader(
                    "specs/egl/" + suffix + ".spec"));
            String egljFilename = "android/opengl/" + suffix + ".java";
            String eglcFilename = "android_opengl_" + suffix + ".cpp";
            PrintStream egljStream =
                new PrintStream(new FileOutputStream("out/" + egljFilename));
            PrintStream eglcStream =
                new PrintStream(new FileOutputStream("out/" + eglcFilename));
            copy("stubs/egl/" + suffix + "Header.java-if", egljStream);
            copy("stubs/egl/" + suffix + "cHeader.cpp", eglcStream);
            EGLCodeEmitter emitter = new EGLCodeEmitter(
                    "android/opengl/" + suffix,
                    checker, egljStream, eglcStream);
            emit(emitter, specReader, egljStream, eglcStream);
            emitter.emitNativeRegistration(
                    "register_android_opengl_jni_" + suffix);
            egljStream.println("}");
            egljStream.close();
            eglcStream.close();
        }
    }
}
