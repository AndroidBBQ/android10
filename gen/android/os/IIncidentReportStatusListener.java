/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.os;
/**
  * Listener for incident report status
  *
  * {@hide}
  */
public interface IIncidentReportStatusListener extends android.os.IInterface
{
  /** Default implementation for IIncidentReportStatusListener. */
  public static class Default implements android.os.IIncidentReportStatusListener
  {
    @Override public void onReportStarted() throws android.os.RemoteException
    {
    }
    @Override public void onReportSectionStatus(int section, int status) throws android.os.RemoteException
    {
    }
    @Override public void onReportFinished() throws android.os.RemoteException
    {
    }
    @Override public void onReportFailed() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.os.IIncidentReportStatusListener
  {
    private static final java.lang.String DESCRIPTOR = "android.os.IIncidentReportStatusListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.os.IIncidentReportStatusListener interface,
     * generating a proxy if needed.
     */
    public static android.os.IIncidentReportStatusListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.os.IIncidentReportStatusListener))) {
        return ((android.os.IIncidentReportStatusListener)iin);
      }
      return new android.os.IIncidentReportStatusListener.Stub.Proxy(obj);
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
        case TRANSACTION_onReportStarted:
        {
          data.enforceInterface(descriptor);
          this.onReportStarted();
          return true;
        }
        case TRANSACTION_onReportSectionStatus:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.onReportSectionStatus(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onReportFinished:
        {
          data.enforceInterface(descriptor);
          this.onReportFinished();
          return true;
        }
        case TRANSACTION_onReportFailed:
        {
          data.enforceInterface(descriptor);
          this.onReportFailed();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.os.IIncidentReportStatusListener
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
      @Override public void onReportStarted() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onReportStarted, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onReportStarted();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onReportSectionStatus(int section, int status) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(section);
          _data.writeInt(status);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onReportSectionStatus, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onReportSectionStatus(section, status);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onReportFinished() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onReportFinished, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onReportFinished();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onReportFailed() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onReportFailed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onReportFailed();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.os.IIncidentReportStatusListener sDefaultImpl;
    }
    static final int TRANSACTION_onReportStarted = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onReportSectionStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onReportFinished = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onReportFailed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(android.os.IIncidentReportStatusListener impl) {
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
    public static android.os.IIncidentReportStatusListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public static final int STATUS_STARTING = 1;
  public static final int STATUS_FINISHED = 2;
  public void onReportStarted() throws android.os.RemoteException;
  public void onReportSectionStatus(int section, int status) throws android.os.RemoteException;
  public void onReportFinished() throws android.os.RemoteException;
  public void onReportFailed() throws android.os.RemoteException;
}
