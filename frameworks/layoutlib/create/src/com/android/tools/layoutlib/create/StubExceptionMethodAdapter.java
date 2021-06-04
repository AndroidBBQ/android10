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

import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.Type;


/**
 * {@link MethodVisitor} that replaces the method the implementation of the method with
 * <code>
 * throw new RuntimeException("Stub!");
 * </code>
 */
public class StubExceptionMethodAdapter extends MethodVisitor {
    private final MethodVisitor mParentVisitor;

    public StubExceptionMethodAdapter(@NotNull MethodVisitor mv, @NotNull String methodName,
            @NotNull Type returnType, @NotNull String invokeSignature, boolean isStatic,
            boolean isNative) {
        super(Main.ASM_VERSION, null);

        mParentVisitor = mv;
    }

    @Override
    public void visitCode() {
        mParentVisitor.visitTypeInsn(Opcodes.NEW, "java/lang/RuntimeException");
        mParentVisitor.visitInsn(Opcodes.DUP);
        mParentVisitor.visitLdcInsn("Stub!");
        mParentVisitor.visitMethodInsn(Opcodes.INVOKESPECIAL, "java/lang/RuntimeException",
                "<init>", "(Ljava" + "/lang/String;)V", false);
        mParentVisitor.visitInsn(Opcodes.ATHROW);
        mParentVisitor.visitMaxs(3, 1);
        mParentVisitor.visitEnd();

    }
}
