/*
 * Copyright (C) 2017 The Android Open Source Project
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

import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.Type;

import static com.android.tools.layoutlib.create.DelegateMethodAdapter.DELEGATE_SUFFIX;

/**
 * This method adapter generates code that will be executed immediately after the constructor
 * has completed. The delegate must implement a <code>constructor_after</code> static method that
 * receives the new instance as parameter.
 * This allows doing operations on the object immediately after construction.
 */
class AfterConstructorMethodAdapter extends MethodVisitor {

    /** Constructor delegate name. */
    private static final String AFTER_CONSTRUCTOR_NAME = "constructor_after";

    /** The internal class name (e.g. <code>com/android/SomeClass$InnerClass</code>.) */
    private final String mClassName;

    /** Logger object. */
    private final Log mLog;

    /**
     * Creates a new {@link AfterConstructorMethodAdapter} that will add code to the constructor
     * to call the delegate with the new created instance.
     * <p/>*
     * @param log The logger object. Must not be null.
     * @param mvOriginal The parent method writer to copy of the original constructor.
     * @param className The internal class name of the class to visit,
     *          e.g. <code>com/android/SomeClass$InnerClass</code>.
     */
    public AfterConstructorMethodAdapter(Log log,
            MethodVisitor mvOriginal,
            String className) {
        super(Main.ASM_VERSION, mvOriginal);
        mLog = log;
        mClassName = className;
    }

    /**
     * Methods that generates the call to the delegate constructor_after method
     */
    private void generateTrampoline() {
        String delegateClassName = mClassName + DELEGATE_SUFFIX;
        delegateClassName =delegateClassName.replace('$', '_');

        // Call constructor_after delegate
        mv.visitVarInsn(Opcodes.ALOAD, 0); // var 0 = this
        mv.visitMethodInsn(Opcodes.INVOKESTATIC, delegateClassName,
                AFTER_CONSTRUCTOR_NAME,
                Type.getMethodDescriptor(Type.VOID_TYPE, Type.getObjectType(mClassName)),
                false);

        mLog.debug("After constructor call for class %s delegated to %s#%s", mClassName,
                delegateClassName, AFTER_CONSTRUCTOR_NAME);
    }

    @Override
    public void visitInsn(int opcode) {
        if (opcode == Opcodes.RETURN) {
            generateTrampoline();
        }

        super.visitInsn(opcode);
    }
}
