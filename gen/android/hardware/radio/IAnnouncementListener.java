/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.hardware.radio;
/** {@hide} */
public interface IAnnouncementListener extends android.os.IInterface
{
  /** Default implementation for IAnnouncementListener. */
  public static class Default implements android.hardware.radio.IAnnouncementListener
  {
    @Override public void onListUpdated(java.util.List<android.hardware.radio.Announcement> activeAnnouncements) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.hardware.radio.IAnnouncementListener
  {
    private static final java.lang.String DESCRIPTOR = "android.hardware.radio.IAnnouncementListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.hardware.radio.IAnnouncementListener interface,
     * generating a proxy if needed.
     */
    public static android.hardware.radio.IAnnouncementListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.hardware.radio.IAnnouncementListener))) {
        return ((android.hardware.radio.IAnnouncementListener)iin);
      }
      return new android.hardware.radio.IAnnouncementListener.Stub.Proxy(obj);
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
        case TRANSACTION_onListUpdated:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.hardware.radio.Announcement> _arg0;
          _arg0 = data.createTypedArrayList(android.hardware.radio.Announcement.CREATOR);
          this.onListUpdated(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.hardware.radio.IAnnouncementListener
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
      @Override public void onListUpdated(java.util.List<android.hardware.radio.Announcement> activeAnnouncements) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedList(activeAnnouncements);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onListUpdated, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onListUpdated(activeAnnouncements);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.hardware.radio.IAnnouncementListener sDefaultImpl;
    }
    static final int TRANSACTION_onListUpdated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.hardware.radio.IAnnouncementListener impl) {
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
    public static android.hardware.radio.IAnnouncementListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onListUpdated(java.util.List<android.hardware.radio.Announcement> activeAnnouncements) throws android.os.RemoteException;
}
