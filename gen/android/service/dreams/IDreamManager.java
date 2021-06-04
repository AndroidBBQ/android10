/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.dreams;
/** @hide */
public interface IDreamManager extends android.os.IInterface
{
  /** Default implementation for IDreamManager. */
  public static class Default implements android.service.dreams.IDreamManager
  {
    @Override public void dream() throws android.os.RemoteException
    {
    }
    @Override public void awaken() throws android.os.RemoteException
    {
    }
    @Override public void setDreamComponents(android.content.ComponentName[] componentNames) throws android.os.RemoteException
    {
    }
    @Override public android.content.ComponentName[] getDreamComponents() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.content.ComponentName getDefaultDreamComponent() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void testDream(android.content.ComponentName componentName) throws android.os.RemoteException
    {
    }
    @Override public boolean isDreaming() throws android.os.RemoteException
    {
      return false;
    }
    @Override public void finishSelf(android.os.IBinder token, boolean immediate) throws android.os.RemoteException
    {
    }
    @Override public void startDozing(android.os.IBinder token, int screenState, int screenBrightness) throws android.os.RemoteException
    {
    }
    @Override public void stopDozing(android.os.IBinder token) throws android.os.RemoteException
    {
    }
    @Override public void forceAmbientDisplayEnabled(boolean enabled) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.dreams.IDreamManager
  {
    private static final java.lang.String DESCRIPTOR = "android.service.dreams.IDreamManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.dreams.IDreamManager interface,
     * generating a proxy if needed.
     */
    public static android.service.dreams.IDreamManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.dreams.IDreamManager))) {
        return ((android.service.dreams.IDreamManager)iin);
      }
      return new android.service.dreams.IDreamManager.Stub.Proxy(obj);
    }
    @Override public android.os.IBinder asBinder()
    {
      return this;
    }
    @Override public boolean onTransact(int code, android.os.Parcel data, android.os.Parcel reply, int flags) throws android.os.RemoteException
    {
      java.lang.String descriptor = DESCRIPTOR;
      switch (code)
      {
        case INTERFACE_TRANSACTION:
        {
          reply.writeString(descriptor);
          return true;
        }
        case TRANSACTION_dream:
        {
          data.enforceInterface(descriptor);
          this.dream();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_awaken:
        {
          data.enforceInterface(descriptor);
          this.awaken();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setDreamComponents:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName[] _arg0;
          _arg0 = data.createTypedArray(android.content.ComponentName.CREATOR);
          this.setDreamComponents(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getDreamComponents:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName[] _result = this.getDreamComponents();
          reply.writeNoException();
          reply.writeTypedArray(_result, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          return true;
        }
        case TRANSACTION_getDefaultDreamComponent:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName _result = this.getDefaultDreamComponent();
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_testDream:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.testDream(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isDreaming:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isDreaming();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_finishSelf:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.finishSelf(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startDozing:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.startDozing(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_stopDozing:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.stopDozing(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_forceAmbientDisplayEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.forceAmbientDisplayEnabled(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.dreams.IDreamManager
    {
      private android.os.IBinder mRemote;
      Proxy(android.os.IBinder remote)
      {
        mRemote = remote;
      }
      @Override public android.os.IBinder asBinder()
      {
        return mRemote;
      }
      public java.lang.String getInterfaceDescriptor()
      {
        return DESCRIPTOR;
      }
      @Override public void dream() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_dream, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dream();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void awaken() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_awaken, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().awaken();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setDreamComponents(android.content.ComponentName[] componentNames) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedArray(componentNames, 0);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDreamComponents, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setDreamComponents(componentNames);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.content.ComponentName[] getDreamComponents() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.ComponentName[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDreamComponents, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDreamComponents();
          }
          _reply.readException();
          _result = _reply.createTypedArray(android.content.ComponentName.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.content.ComponentName getDefaultDreamComponent() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.ComponentName _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDefaultDreamComponent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDefaultDreamComponent();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.content.ComponentName.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void testDream(android.content.ComponentName componentName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((componentName!=null)) {
            _data.writeInt(1);
            componentName.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_testDream, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().testDream(componentName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isDreaming() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isDreaming, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isDreaming();
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void finishSelf(android.os.IBinder token, boolean immediate) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeInt(((immediate)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_finishSelf, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().finishSelf(token, immediate);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void startDozing(android.os.IBinder token, int screenState, int screenBrightness) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeInt(screenState);
          _data.writeInt(screenBrightness);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startDozing, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startDozing(token, screenState, screenBrightness);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void stopDozing(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopDozing, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopDozing(token);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void forceAmbientDisplayEnabled(boolean enabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_forceAmbientDisplayEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().forceAmbientDisplayEnabled(enabled);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.service.dreams.IDreamManager sDefaultImpl;
    }
    static final int TRANSACTION_dream = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_awaken = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_setDreamComponents = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getDreamComponents = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getDefaultDreamComponent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_testDream = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_isDreaming = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_finishSelf = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_startDozing = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_stopDozing = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_forceAmbientDisplayEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    public static boolean setDefaultImpl(android.service.dreams.IDreamManager impl) {
      // Only one user of this interface can use this function
      // at a time. This is a heuristic to detect if two different
      // users in the same process use this function.
      if (Stub.Proxy.sDefaultImpl != null) {
        throw new IllegalStateException("setDefaultImpl() called twice");
      }
      if (impl != null) {
        Stub.Proxy.sDefaultImpl = impl;
        return true;
      }
      return false;
    }
    public static android.service.dreams.IDreamManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/service/dreams/IDreamManager.aidl:26:1:26:25")
  public void dream() throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/service/dreams/IDreamManager.aidl:28:1:28:25")
  public void awaken() throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/service/dreams/IDreamManager.aidl:30:1:30:25")
  public void setDreamComponents(android.content.ComponentName[] componentNames) throws android.os.RemoteException;
  public android.content.ComponentName[] getDreamComponents() throws android.os.RemoteException;
  public android.content.ComponentName getDefaultDreamComponent() throws android.os.RemoteException;
  public void testDream(android.content.ComponentName componentName) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/service/dreams/IDreamManager.aidl:35:1:35:25")
  public boolean isDreaming() throws android.os.RemoteException;
  public void finishSelf(android.os.IBinder token, boolean immediate) throws android.os.RemoteException;
  public void startDozing(android.os.IBinder token, int screenState, int screenBrightness) throws android.os.RemoteException;
  public void stopDozing(android.os.IBinder token) throws android.os.RemoteException;
  public void forceAmbientDisplayEnabled(boolean enabled) throws android.os.RemoteException;
}
