/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app.contentsuggestions;
/** @hide */
public interface ISelectionsCallback extends android.os.IInterface
{
  /** Default implementation for ISelectionsCallback. */
  public static class Default implements android.app.contentsuggestions.ISelectionsCallback
  {
    @Override public void onContentSelectionsAvailable(int statusCode, java.util.List<android.app.contentsuggestions.ContentSelection> selections) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.contentsuggestions.ISelectionsCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.app.contentsuggestions.ISelectionsCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.contentsuggestions.ISelectionsCallback interface,
     * generating a proxy if needed.
     */
    public static android.app.contentsuggestions.ISelectionsCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.contentsuggestions.ISelectionsCallback))) {
        return ((android.app.contentsuggestions.ISelectionsCallback)iin);
      }
      return new android.app.contentsuggestions.ISelectionsCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onContentSelectionsAvailable:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.util.List<android.app.contentsuggestions.ContentSelection> _arg1;
          _arg1 = data.createTypedArrayList(android.app.contentsuggestions.ContentSelection.CREATOR);
          this.onContentSelectionsAvailable(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.contentsuggestions.ISelectionsCallback
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
      @Override public void onContentSelectionsAvailable(int statusCode, java.util.List<android.app.contentsuggestions.ContentSelection> selections) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(statusCode);
          _data.writeTypedList(selections);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onContentSelectionsAvailable, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onContentSelectionsAvailable(statusCode, selections);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.app.contentsuggestions.ISelectionsCallback sDefaultImpl;
    }
    static final int TRANSACTION_onContentSelectionsAvailable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.app.contentsuggestions.ISelectionsCallback impl) {
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
    public static android.app.contentsuggestions.ISelectionsCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onContentSelectionsAvailable(int statusCode, java.util.List<android.app.contentsuggestions.ContentSelection> selections) throws android.os.RemoteException;
}
