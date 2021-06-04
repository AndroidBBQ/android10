/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app.contentsuggestions;
/** @hide */
public interface IClassificationsCallback extends android.os.IInterface
{
  /** Default implementation for IClassificationsCallback. */
  public static class Default implements android.app.contentsuggestions.IClassificationsCallback
  {
    @Override public void onContentClassificationsAvailable(int statusCode, java.util.List<android.app.contentsuggestions.ContentClassification> classifications) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.contentsuggestions.IClassificationsCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.app.contentsuggestions.IClassificationsCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.contentsuggestions.IClassificationsCallback interface,
     * generating a proxy if needed.
     */
    public static android.app.contentsuggestions.IClassificationsCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.contentsuggestions.IClassificationsCallback))) {
        return ((android.app.contentsuggestions.IClassificationsCallback)iin);
      }
      return new android.app.contentsuggestions.IClassificationsCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onContentClassificationsAvailable:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.util.List<android.app.contentsuggestions.ContentClassification> _arg1;
          _arg1 = data.createTypedArrayList(android.app.contentsuggestions.ContentClassification.CREATOR);
          this.onContentClassificationsAvailable(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.contentsuggestions.IClassificationsCallback
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
      @Override public void onContentClassificationsAvailable(int statusCode, java.util.List<android.app.contentsuggestions.ContentClassification> classifications) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(statusCode);
          _data.writeTypedList(classifications);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onContentClassificationsAvailable, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onContentClassificationsAvailable(statusCode, classifications);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.app.contentsuggestions.IClassificationsCallback sDefaultImpl;
    }
    static final int TRANSACTION_onContentClassificationsAvailable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.app.contentsuggestions.IClassificationsCallback impl) {
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
    public static android.app.contentsuggestions.IClassificationsCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onContentClassificationsAvailable(int statusCode, java.util.List<android.app.contentsuggestions.ContentClassification> classifications) throws android.os.RemoteException;
}
