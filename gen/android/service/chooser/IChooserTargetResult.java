/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.chooser;
/**
 * @hide
 */
public interface IChooserTargetResult extends android.os.IInterface
{
  /** Default implementation for IChooserTargetResult. */
  public static class Default implements android.service.chooser.IChooserTargetResult
  {
    @Override public void sendResult(java.util.List<android.service.chooser.ChooserTarget> targets) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.chooser.IChooserTargetResult
  {
    private static final java.lang.String DESCRIPTOR = "android.service.chooser.IChooserTargetResult";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.chooser.IChooserTargetResult interface,
     * generating a proxy if needed.
     */
    public static android.service.chooser.IChooserTargetResult asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.chooser.IChooserTargetResult))) {
        return ((android.service.chooser.IChooserTargetResult)iin);
      }
      return new android.service.chooser.IChooserTargetResult.Stub.Proxy(obj);
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
        case TRANSACTION_sendResult:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.service.chooser.ChooserTarget> _arg0;
          _arg0 = data.createTypedArrayList(android.service.chooser.ChooserTarget.CREATOR);
          this.sendResult(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.chooser.IChooserTargetResult
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
      @Override public void sendResult(java.util.List<android.service.chooser.ChooserTarget> targets) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedList(targets);
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendResult, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendResult(targets);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.service.chooser.IChooserTargetResult sDefaultImpl;
    }
    static final int TRANSACTION_sendResult = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.service.chooser.IChooserTargetResult impl) {
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
    public static android.service.chooser.IChooserTargetResult getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void sendResult(java.util.List<android.service.chooser.ChooserTarget> targets) throws android.os.RemoteException;
}
