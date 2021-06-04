/*
 * Copyright (C) 2018 The Android Open Source Project
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

package com.android.tools.layoutlib.create;

import com.android.tools.layoutlib.annotations.NotNull;

import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Map;
import java.util.Map.Entry;
import java.util.function.Function;
import java.util.jar.JarEntry;
import java.util.jar.JarOutputStream;

public class JarUtil {
    private JarUtil() {
    }

    /**
     * Writes the JAR file.
     *
     * @param outStream The file output stream were to write the JAR.
     * @param all The map of all classes to output.
     * @param transform Transform to apply to the class files
     *
     * @throws IOException if an I/O error has occurred
     */
    public static void createJar(@NotNull FileOutputStream outStream,
            @NotNull Map<String, byte[]> all, @NotNull Function<byte[], byte[]> transform)
            throws IOException {
        JarOutputStream jar = new JarOutputStream(outStream);
        for (Entry<String, byte[]> entry : all.entrySet()) {
            String name = entry.getKey();
            JarEntry jar_entry = new JarEntry(name);
            jar.putNextEntry(jar_entry);
            if (name.endsWith(".class")) {
                jar.write(transform.apply(entry.getValue()));
            } else {
                // This is just a file,
                jar.write(entry.getValue());
            }
            jar.closeEntry();
        }
        jar.flush();
        jar.close();
    }

    /**
     * Writes the JAR file.
     *
     * @param outStream The file output stream were to write the JAR.
     * @param all The map of all classes to output.
     *
     * @throws IOException if an I/O error has occurred
     */
    public static void createJar(@NotNull FileOutputStream outStream,
            @NotNull Map<String, byte[]> all) throws IOException {
        createJar(outStream, all, Function.identity());
    }
}
