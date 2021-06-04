/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.print;
/**
 * Callbacks for communication with the print spooler service.
 *
 * @see android.print.IPrintSpoolerService
 *
 * @hide
 */
public interface IPrintSpoolerCallbacks extends android.os.IInterface
{
  /** Default implementation for IPrintSpoolerCallbacks. */
  public static class Default implements android.print.IPrintSpoolerCallbacks
  {
    @Override public void onGetPrintJobInfosResult(java.util.List<android.print.PrintJobInfo> printJob, int sequence) throws android.os.RemoteException
    {
    }
    @Override public void onCancelPrintJobResult(boolean canceled, int sequence) throws android.os.RemoteException
    {
    }
    @Override public void onSetPrintJobStateResult(boolean success, int sequence) throws android.os.RemoteException
    {
    }
    @Override public void onSetPrintJobTagResult(boolean success, int sequence) throws android.os.RemoteException
    {
    }
    @Override public void onGetPrintJobInfoResult(android.print.PrintJobInfo printJob, int sequence) throws android.os.RemoteException
    {
    }
    /**
         * Deliver the result of a request of a custom printer icon.
         *
         * @param icon the icon that was retrieved, or null if no icon could be
         *             found
         * @param sequence the sequence number of the call to get the icon
         */
    @Override public void onGetCustomPrinterIconResult(android.graphics.drawable.Icon icon, int sequence) throws android.os.RemoteException
    {
    }
    /**
         * Declare that the print spooler cached a custom printer icon.
         *
         * @param sequence the sequence number of the call to cache the icon
         */
    @Override public void onCustomPrinterIconCached(int sequence) throws android.os.RemoteException
    {
    }
    /**
         * Declare that the custom printer icon cache was cleared.
         *
         * @param sequence the sequence number of the call to clear the cache
         */
    @Override public void customPrinterIconCacheCleared(int sequence) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.print.IPrintSpoolerCallbacks
  {
    private static final java.lang.String DESCRIPTOR = "android.print.IPrintSpoolerCallbacks";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.print.IPrintSpoolerCallbacks interface,
     * generating a proxy if needed.
     */
    public static android.print.IPrintSpoolerCallbacks asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.print.IPrintSpoolerCallbacks))) {
        return ((android.print.IPrintSpoolerCallbacks)iin);
      }
      return new android.print.IPrintSpoolerCallbacks.Stub.Proxy(obj);
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
        case TRANSACTION_onGetPrintJobInfosResult:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.print.PrintJobInfo> _arg0;
          _arg0 = data.createTypedArrayList(android.print.PrintJobInfo.CREATOR);
          int _arg1;
          _arg1 = data.readInt();
          this.onGetPrintJobInfosResult(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onCancelPrintJobResult:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          int _arg1;
          _arg1 = data.readInt();
          this.onCancelPrintJobResult(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onSetPrintJobStateResult:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          int _arg1;
          _arg1 = data.readInt();
          this.onSetPrintJobStateResult(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onSetPrintJobTagResult:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          int _arg1;
          _arg1 = data.readInt();
          this.onSetPrintJobTagResult(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onGetPrintJobInfoResult:
        {
          data.enforceInterface(descriptor);
          android.print.PrintJobInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.print.PrintJobInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          this.onGetPrintJobInfoResult(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onGetCustomPrinterIconResult:
        {
          data.enforceInterface(descriptor);
          android.graphics.drawable.Icon _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.graphics.drawable.Icon.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          this.onGetCustomPrinterIconResult(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onCustomPrinterIconCached:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onCustomPrinterIconCached(_arg0);
          return true;
        }
        case TRANSACTION_customPrinterIconCacheCleared:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.customPrinterIconCacheCleared(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.print.IPrintSpoolerCallbacks
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
      @Override public void onGetPrintJobInfosResult(java.util.List<android.print.PrintJobInfo> printJob, int sequence) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedList(printJob);
          _data.writeInt(sequence);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onGetPrintJobInfosResult, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onGetPrintJobInfosResult(printJob, sequence);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onCancelPrintJobResult(boolean canceled, int sequence) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((canceled)?(1):(0)));
          _data.writeInt(sequence);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onCancelPrintJobResult, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onCancelPrintJobResult(canceled, sequence);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onSetPrintJobStateResult(boolean success, int sequence) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((success)?(1):(0)));
          _data.writeInt(sequence);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSetPrintJobStateResult, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSetPrintJobStateResult(success, sequence);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onSetPrintJobTagResult(boolean success, int sequence) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((success)?(1):(0)));
          _data.writeInt(sequence);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSetPrintJobTagResult, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSetPrintJobTagResult(success, sequence);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onGetPrintJobInfoResult(android.print.PrintJobInfo printJob, int sequence) throws android.os.RemoteException
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
          _data.writeInt(sequence);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onGetPrintJobInfoResult, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onGetPrintJobInfoResult(printJob, sequence);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Deliver the result of a request of a custom printer icon.
           *
           * @param icon the icon that was retrieved, or null if no icon could be
           *             found
           * @param sequence the sequence number of the call to get the icon
           */
      @Override public void onGetCustomPrinterIconResult(android.graphics.drawable.Icon icon, int sequence) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((icon!=null)) {
            _data.writeInt(1);
            icon.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(sequence);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onGetCustomPrinterIconResult, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onGetCustomPrinterIconResult(icon, sequence);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Declare that the print spooler cached a custom printer icon.
           *
           * @param sequence the sequence number of the call to cache the icon
           */
      @Override public void onCustomPrinterIconCached(int sequence) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(sequence);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onCustomPrinterIconCached, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onCustomPrinterIconCached(sequence);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Declare that the custom printer icon cache was cleared.
           *
           * @param sequence the sequence number of the call to clear the cache
           */
      @Override public void customPrinterIconCacheCleared(int sequence) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(sequence);
          boolean _status = mRemote.transact(Stub.TRANSACTION_customPrinterIconCacheCleared, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().customPrinterIconCacheCleared(sequence);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.print.IPrintSpoolerCallbacks sDefaultImpl;
    }
    static final int TRANSACTION_onGetPrintJobInfosResult = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onCancelPrintJobResult = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onSetPrintJobStateResult = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onSetPrintJobTagResult = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onGetPrintJobInfoResult = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_onGetCustomPrinterIconResult = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_onCustomPrinterIconCached = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_customPrinterIconCacheCleared = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    public static boolean setDefaultImpl(android.print.IPrintSpoolerCallbacks impl) {
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
    public static android.print.IPrintSpoolerCallbacks getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onGetPrintJobInfosResult(java.util.List<android.print.PrintJobInfo> printJob, int sequence) throws android.os.RemoteException;
  public void onCancelPrintJobResult(boolean canceled, int sequence) throws android.os.RemoteException;
  public void onSetPrintJobStateResult(boolean success, int sequence) throws android.os.RemoteException;
  public void onSetPrintJobTagResult(boolean success, int sequence) throws android.os.RemoteException;
  public void onGetPrintJobInfoResult(android.print.PrintJobInfo printJob, int sequence) throws android.os.RemoteException;
  /**
       * Deliver the result of a request of a custom printer icon.
       *
       * @param icon the icon that was retrieved, or null if no icon could be
       *             found
       * @param sequence the sequence number of the call to get the icon
       */
  public void onGetCustomPrinterIconResult(android.graphics.drawable.Icon icon, int sequence) throws android.os.RemoteException;
  /**
       * Declare that the print spooler cached a custom printer icon.
       *
       * @param sequence the sequence number of the call to cache the icon
       */
  public void onCustomPrinterIconCached(int sequence) throws android.os.RemoteException;
  /**
       * Declare that the custom printer icon cache was cleared.
       *
       * @param sequence the sequence number of the call to clear the cache
       */
  public void customPrinterIconCacheCleared(int sequence) throws android.os.RemoteException;
}
