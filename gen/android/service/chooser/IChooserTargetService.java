/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.chooser;
/**
 * @hide
 */
public interface IChooserTargetService extends android.os.IInterface
{
  /** Default implementation for IChooserTargetService. */
  public static class Default implements android.service.chooser.IChooserTargetService
  {
    @Override public void getChooserTargets(android.content.ComponentName targetComponentName, android.content.IntentFilter matchedFilter, android.service.chooser.IChooserTargetResult result) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.chooser.IChooserTargetService
  {
    private static final java.lang.String DESCRIPTOR = "android.service.chooser.IChooserTargetService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.chooser.IChooserTargetService interface,
     * generating a proxy if needed.
     */
    public static android.service.chooser.IChooserTargetService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.chooser.IChooserTargetService))) {
        return ((android.service.chooser.IChooserTargetService)iin);
      }
      return new android.service.chooser.IChooserTargetService.Stub.Proxy(obj);
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
        case TRANSACTION_getChooserTargets:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.content.IntentFilter _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.IntentFilter.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.service.chooser.IChooserTargetResult _arg2;
          _arg2 = android.service.chooser.IChooserTargetResult.Stub.asInterface(data.readStrongBinder());
          this.getChooserTargets(_arg0, _arg1, _arg2);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.chooser.IChooserTargetService
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
      @Override public void getChooserTargets(android.content.ComponentName targetComponentName, android.content.IntentFilter matchedFilter, android.service.chooser.IChooserTargetResult result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((targetComponentName!=null)) {
            _data.writeInt(1);
            targetComponentName.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((matchedFilter!=null)) {
            _data.writeInt(1);
            matchedFilter.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((result!=null))?(result.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getChooserTargets, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getChooserTargets(targetComponentName, matchedFilter, result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.service.chooser.IChooserTargetService sDefaultImpl;
    }
    static final int TRANSACTION_getChooserTargets = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.service.chooser.IChooserTargetService impl) {
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
    public static android.service.chooser.IChooserTargetService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void getChooserTargets(android.content.ComponentName targetComponentName, android.content.IntentFilter matchedFilter, android.service.chooser.IChooserTargetResult result) throws android.os.RemoteException;
}
