/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app;
/** @hide */
public interface IAssistDataReceiver extends android.os.IInterface
{
  /** Default implementation for IAssistDataReceiver. */
  public static class Default implements android.app.IAssistDataReceiver
  {
    @Override public void onHandleAssistData(android.os.Bundle resultData) throws android.os.RemoteException
    {
    }
    @Override public void onHandleAssistScreenshot(android.graphics.Bitmap screenshot) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.IAssistDataReceiver
  {
    private static final java.lang.String DESCRIPTOR = "android.app.IAssistDataReceiver";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.IAssistDataReceiver interface,
     * generating a proxy if needed.
     */
    public static android.app.IAssistDataReceiver asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.IAssistDataReceiver))) {
        return ((android.app.IAssistDataReceiver)iin);
      }
      return new android.app.IAssistDataReceiver.Stub.Proxy(obj);
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
        case TRANSACTION_onHandleAssistData:
        {
          data.enforceInterface(descriptor);
          android.os.Bundle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onHandleAssistData(_arg0);
          return true;
        }
        case TRANSACTION_onHandleAssistScreenshot:
        {
          data.enforceInterface(descriptor);
          android.graphics.Bitmap _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.graphics.Bitmap.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onHandleAssistScreenshot(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.IAssistDataReceiver
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
      @Override public void onHandleAssistData(android.os.Bundle resultData) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((resultData!=null)) {
            _data.writeInt(1);
            resultData.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onHandleAssistData, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onHandleAssistData(resultData);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onHandleAssistScreenshot(android.graphics.Bitmap screenshot) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((screenshot!=null)) {
            _data.writeInt(1);
            screenshot.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onHandleAssistScreenshot, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onHandleAssistScreenshot(screenshot);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.app.IAssistDataReceiver sDefaultImpl;
    }
    static final int TRANSACTION_onHandleAssistData = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onHandleAssistScreenshot = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.app.IAssistDataReceiver impl) {
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
    public static android.app.IAssistDataReceiver getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IAssistDataReceiver.aidl:24:1:24:25")
  public void onHandleAssistData(android.os.Bundle resultData) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/IAssistDataReceiver.aidl:26:1:26:25")
  public void onHandleAssistScreenshot(android.graphics.Bitmap screenshot) throws android.os.RemoteException;
}
