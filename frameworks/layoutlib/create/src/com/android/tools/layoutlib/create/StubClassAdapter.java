/*
 * Copyright (C) 2008 The Android Open Source Project
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
import com.android.tools.layoutlib.annotations.Nullable;
import com.android.tools.layoutlib.annotations.VisibleForTesting;

import org.objectweb.asm.ClassReader;
import org.objectweb.asm.ClassVisitor;
import org.objectweb.asm.ClassWriter;
import org.objectweb.asm.FieldVisitor;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.Type;

import java.util.Collections;
import java.util.Set;

/**
 * Class adapter that can stub some or all of the methods of the class.
 */
class StubClassAdapter extends ClassVisitor {
    public interface MethodVisitorFactory {
        @NotNull
        MethodVisitor create(@NotNull MethodVisitor mv,
                @NotNull String methodName,
                @NotNull Type returnType,
                @NotNull String invokeSignature,
                boolean isStatic, boolean isNative);
    }

    public static class Builder {
        private Log mLogger;
        private Set<String> mDeleteReturns;
        private String mClassName;
        private ClassVisitor mCv;
        private boolean mStubNativesOnly;
        private boolean mRemoveStaticInitializers;
        private boolean mRemovePrivates;
        private MethodVisitorFactory mMethodVisitorFactory;

        private Builder(@NotNull Log log, @NotNull ClassVisitor classVisitor) {
            mLogger = log;
            mCv = classVisitor;
        }

        @NotNull
        public Builder withDeleteReturns(@Nullable Set<String> deleteReturns) {
            mDeleteReturns = deleteReturns;
            return this;
        }

        @NotNull
        public Builder withNewClassName(@Nullable String className) {
            mClassName = className;
            return this;
        }

        public Builder useOnlyStubNative(boolean stubNativesOnly) {
            mStubNativesOnly = stubNativesOnly;
            return this;
        }

        @NotNull
        public Builder withMethodVisitorFactory(@Nullable MethodVisitorFactory factory) {
            mMethodVisitorFactory = factory;
            return this;
        }

        @NotNull
        public Builder removePrivates() {
            mRemovePrivates = true;
            return this;
        }

        @NotNull
        public Builder removeStaticInitializers() {
            mRemoveStaticInitializers = true;
            return this;
        }

        public StubClassAdapter build() {
            return new StubClassAdapter(mLogger,
                    mDeleteReturns != null ? mDeleteReturns : Collections.emptySet(),
                    mClassName,
                    mCv,
                    mMethodVisitorFactory != null ? mMethodVisitorFactory : StubCallMethodAdapter::new,
                    mStubNativesOnly,
                    mRemovePrivates,
                    mRemoveStaticInitializers);
        }
    }

    /** True if all methods should be stubbed, false if only native ones must be stubbed. */
    private final boolean mStubAll;
    /** True if the class is an interface. */
    private boolean mIsInterface;
    private final String mClassName;
    private final Log mLog;
    private final Set<String> mDeleteReturns;
    private final MethodVisitorFactory mMethodVisitorFactory;
    private final boolean mRemovePrivates;
    private final boolean mRemoveStaticInitalizers;


    @NotNull
    public static StubClassAdapter.Builder builder(@NotNull Log log,
            @NotNull ClassVisitor classVisitor) {
        return new Builder(log, classVisitor);
    }
    /**
     * Creates a new class adapter that will stub some or all methods.
     * @param deleteReturns list of types that trigger the deletion of methods returning them.
     * @param className Optional new name for the class being modified
     * @param cv The parent class writer visitor
     * @param stubNativesOnly True if only native methods should be stubbed. False if all
     * @param removePrivates If true, all private methods and fields will be removed
     * @param removeStaticInitializers If true, static initializers will be removed
     */
    private StubClassAdapter(@NotNull Log logger,
            @NotNull Set<String> deleteReturns, @Nullable String className,
            @NotNull ClassVisitor cv, @NotNull MethodVisitorFactory methodVisitorFactory,
            boolean stubNativesOnly, boolean removePrivates, boolean removeStaticInitializers) {
        super(Main.ASM_VERSION, cv);
        mLog = logger;
        mClassName = className;
        mStubAll = !stubNativesOnly;
        mIsInterface = false;
        mDeleteReturns = deleteReturns;
        mMethodVisitorFactory = methodVisitorFactory;
        mRemovePrivates = removePrivates;
        mRemoveStaticInitalizers = removeStaticInitializers;
    }

    /**
     * Utility method that receives a class in serialized form and returns the stubbed version.
     */
    @VisibleForTesting
    @NotNull
    static byte[] stubClass(@NotNull Log log, @NotNull byte[] bytes,
            @Nullable String newName) {
        ClassReader classReader = new ClassReader(bytes);
        ClassWriter classWriter = new ClassWriter(0);

        // We replace every method with a Stub exception throw and remove all private
        // methods and static initializers since we only care about the interfaces.
        ClassVisitor cv = StubClassAdapter.builder(log, classWriter)
                .withNewClassName(newName)
                .withMethodVisitorFactory(StubExceptionMethodAdapter::new)
                .removePrivates()
                .removeStaticInitializers()
                .build();

        classReader.accept(cv, 0);

        return classWriter.toByteArray();
    }

    /**
     * Utility method that receives a class in serialized form and returns the stubbed version.
     */
    @NotNull
    public static byte[] stubClass(@NotNull Log log, @NotNull byte[] bytes) {
        return stubClass(log, bytes, null);
    }

    /* Visits the class header. */
    @Override
    public void visit(int version, int access, String name,
            String signature, String superName, String[] interfaces) {

        if (mClassName != null) {
            // This class might be being renamed.
            name = mClassName;
        }

        // remove final
        access = access & ~Opcodes.ACC_FINAL;
        // note: leave abstract classes as such
        // don't try to implement stub for interfaces

        mIsInterface = ((access & Opcodes.ACC_INTERFACE) != 0);
        super.visit(version, access, name, signature, superName, interfaces);
    }

    /* Visits the header of an inner class. */
    @Override
    public void visitInnerClass(String name, String outerName, String innerName, int access) {
        // remove final
        access = access & ~Opcodes.ACC_FINAL;
        // note: leave abstract classes as such
        // don't try to implement stub for interfaces

        super.visitInnerClass(name, outerName, innerName, access);
    }

    /* Visits a method. */
    @Override
    public MethodVisitor visitMethod(int access, String name, String desc,
            String signature, String[] exceptions) {
        if (mRemovePrivates && (access & Opcodes.ACC_PRIVATE) != 0) {
            return null;
        }

        if (mRemoveStaticInitalizers && "<clinit>".equals(name)) {
            return null;
        }

        if (mDeleteReturns != null) {
            Type t = Type.getReturnType(desc);
            if (t.getSort() == Type.OBJECT) {
                String returnType = t.getInternalName();
                if (returnType != null) {
                    if (mDeleteReturns.contains(returnType)) {
                        return null;
                    }
                }
            }
        }

        String methodSignature = mClassName != null ?
                mClassName.replace('/', '.') + "#" + name :
                signature;

        // remove final
        access = access & ~Opcodes.ACC_FINAL;

        // stub this method if they are all to be stubbed or if it is a native method
        // and don't try to stub interfaces nor abstract non-native methods.
        if (!mIsInterface &&
            ((access & (Opcodes.ACC_ABSTRACT | Opcodes.ACC_NATIVE)) != Opcodes.ACC_ABSTRACT) &&
            (mStubAll ||
             (access & Opcodes.ACC_NATIVE) != 0)) {

            boolean isStatic = (access & Opcodes.ACC_STATIC) != 0;
            boolean isNative = (access & Opcodes.ACC_NATIVE) != 0;

            // remove abstract, final and native
            access = access & ~(Opcodes.ACC_ABSTRACT | Opcodes.ACC_FINAL | Opcodes.ACC_NATIVE);

            String invokeSignature = methodSignature + desc;
            mLog.debug("  Stub: %s (%s)", invokeSignature, isNative ? "native" : "");

            MethodVisitor mw = super.visitMethod(access, name, desc, signature, exceptions);
            return mMethodVisitorFactory.create(mw, name, returnType(desc), invokeSignature,
                    isStatic, isNative);

        } else {
            mLog.debug("  Keep: %s %s", name, desc);
            return super.visitMethod(access, name, desc, signature, exceptions);
        }
    }

    @Override
    public FieldVisitor visitField(int access, String name, String desc, String signature,
            Object value) {
        if (mRemovePrivates && (access & Opcodes.ACC_PRIVATE) != 0) {
            return null;
        }

        return super.visitField(access, name, desc, signature, value);
    }

    /**
     * Extracts the return {@link Type} of this descriptor.
     */
    private static Type returnType(String desc) {
        if (desc != null) {
            try {
                return Type.getReturnType(desc);
            } catch (ArrayIndexOutOfBoundsException e) {
                // ignore, not a valid type.
            }
        }
        return null;
    }
}
