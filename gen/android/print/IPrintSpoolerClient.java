/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.print;
/**
 * Interface for receiving interesting state updates from the print spooler.
 *
 * @hide
 */
public interface IPrintSpoolerClient extends android.os.IInterface
{
  /** Default implementation for IPrintSpoolerClient. */
  public static class Default implements android.print.IPrintSpoolerClient
  {
    @Override public void onPrintJobQueued(android.print.PrintJobInfo printJob) throws android.os.RemoteException
    {
    }
    @Override public void onAllPrintJobsForServiceHandled(android.content.ComponentName printService) throws android.os.RemoteException
    {
    }
    @Override public void onAllPrintJobsHandled() throws android.os.RemoteException
    {
    }
    @Override public void onPrintJobStateChanged(android.print.PrintJobInfo printJob) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.print.IPrintSpoolerClient
  {
    private static final java.lang.String DESCRIPTOR = "android.print.IPrintSpoolerClient";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.print.IPrintSpoolerClient interface,
     * generating a proxy if needed.
     */
    public static android.print.IPrintSpoolerClient asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.print.IPrintSpoolerClient))) {
        return ((android.print.IPrintSpoolerClient)iin);
      }
      return new android.print.IPrintSpoolerClient.Stub.Proxy(obj);
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
        case TRANSACTION_onPrintJobQueued:
        {
          data.enforceInterface(descriptor);
          android.print.PrintJobInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.print.PrintJobInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onPrintJobQueued(_arg0);
          return true;
        }
        case TRANSACTION_onAllPrintJobsForServiceHandled:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onAllPrintJobsForServiceHandled(_arg0);
          return true;
        }
        case TRANSACTION_onAllPrintJobsHandled:
        {
          data.enforceInterface(descriptor);
          this.onAllPrintJobsHandled();
          return true;
        }
        case TRANSACTION_onPrintJobStateChanged:
        {
          data.enforceInterface(descriptor);
          android.print.PrintJobInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.print.PrintJobInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onPrintJobStateChanged(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.print.IPrintSpoolerClient
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
      @Override public void onPrintJobQueued(android.print.PrintJobInfo printJob) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((printJob!=null)) {
            _data.writeInt(1);
            printJob.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPrintJobQueued, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPrintJobQueued(printJob);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onAllPrintJobsForServiceHandled(android.content.ComponentName printService) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((printService!=null)) {
            _data.writeInt(1);
            printService.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onAllPrintJobsForServiceHandled, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onAllPrintJobsForServiceHandled(printService);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onAllPrintJobsHandled() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onAllPrintJobsHandled, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onAllPrintJobsHandled();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onPrintJobStateChanged(android.print.PrintJobInfo printJob) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((printJob!=null)) {
            _data.writeInt(1);
            printJob.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPrintJobStateChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPrintJobStateChanged(printJob);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.print.IPrintSpoolerClient sDefaultImpl;
    }
    static final int TRANSACTION_onPrintJobQueued = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onAllPrintJobsForServiceHandled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onAllPrintJobsHandled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onPrintJobStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(android.print.IPrintSpoolerClient impl) {
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
    public static android.print.IPrintSpoolerClient getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onPrintJobQueued(android.print.PrintJobInfo printJob) throws android.os.RemoteException;
  public void onAllPrintJobsForServiceHandled(android.content.ComponentName printService) throws android.os.RemoteException;
  public void onAllPrintJobsHandled() throws android.os.RemoteException;
  public void onPrintJobStateChanged(android.print.PrintJobInfo printJob) throws android.os.RemoteException;
}
