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

import org.objectweb.asm.ClassVisitor;

/**
 * Class adapter that modifies the file version of classes to be
 * below a certain maximum version.
 */
public class ChangeFileVersionAdapter extends ClassVisitor {
    private final Log mLog;
    private final int mMaxVersion;

    public ChangeFileVersionAdapter(Log logger, int maxVersion, ClassVisitor cv) {
        super(Main.ASM_VERSION, cv);
        mLog = logger;
        mMaxVersion = maxVersion;
    }

    @Override
    public void visit(int version, int access, String name, String signature, String superName,
            String[] interfaces) {
        int classFileVersion = version;
        if (classFileVersion > mMaxVersion) {
            classFileVersion = mMaxVersion;
            mLog.debug("Class %s has had its file version changed from %d to %d", name, version,
                    classFileVersion);
        }
        super.visit(classFileVersion, access, name, signature, superName, interfaces);
    }
}
