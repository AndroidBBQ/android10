/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.print;
/**
 * Interface for communication with the print spooler service.
 *
 * @see android.print.IPrintSpoolerCallbacks
 *
 * @hide
 */
public interface IPrintSpooler extends android.os.IInterface
{
  /** Default implementation for IPrintSpooler. */
  public static class Default implements android.print.IPrintSpooler
  {
    @Override public void removeObsoletePrintJobs() throws android.os.RemoteException
    {
    }
    @Override public void getPrintJobInfos(android.print.IPrintSpoolerCallbacks callback, android.content.ComponentName componentName, int state, int appId, int sequence) throws android.os.RemoteException
    {
    }
    @Override public void getPrintJobInfo(android.print.PrintJobId printJobId, android.print.IPrintSpoolerCallbacks callback, int appId, int sequence) throws android.os.RemoteException
    {
    }
    @Override public void createPrintJob(android.print.PrintJobInfo printJob) throws android.os.RemoteException
    {
    }
    @Override public void setPrintJobState(android.print.PrintJobId printJobId, int status, java.lang.String stateReason, android.print.IPrintSpoolerCallbacks callback, int sequence) throws android.os.RemoteException
    {
    }
    /**
         * Set the progress of this print job
         *
         * @param printJobId The print job to update
         * @param progress The new progress
         */
    @Override public void setProgress(android.print.PrintJobId printJobId, float progress) throws android.os.RemoteException
    {
    }
    /**
         * Set the status of this print job
         *
         * @param printJobId The print job to update
         * @param status The new status, can be null
         */
    @Override public void setStatus(android.print.PrintJobId printJobId, java.lang.CharSequence status) throws android.os.RemoteException
    {
    }
    /**
         * Set the status of this print job
         *
         * @param printJobId The print job to update
         * @param status The new status as a string resource
         * @param appPackageName App package name the resource belongs to
         */
    @Override public void setStatusRes(android.print.PrintJobId printJobId, int status, java.lang.CharSequence appPackageName) throws android.os.RemoteException
    {
    }
    /**
         * Handle that a custom icon for a printer was loaded.
         *
         * @param printerId the id of the printer the icon belongs to
         * @param icon the icon that was loaded
         * @param callbacks the callback to call once icon is stored in case
         * @param sequence the sequence number of the call
         * @see android.print.PrinterInfo.Builder#setHasCustomPrinterIcon()
         */
    @Override public void onCustomPrinterIconLoaded(android.print.PrinterId printerId, android.graphics.drawable.Icon icon, android.print.IPrintSpoolerCallbacks callbacks, int sequence) throws android.os.RemoteException
    {
    }
    /**
         * Get the custom icon for a printer. If the icon is not cached, the icon is
         * requested asynchronously. Once it is available the printer is updated.
         *
         * @param printerId the id of the printer the icon should be loaded for
         * @param callbacks the callback to call once icon is retrieved
         * @param sequence the sequence number of the call
         * @see android.print.PrinterInfo.Builder#setHasCustomPrinterIcon()
         */
    @Override public void getCustomPrinterIcon(android.print.PrinterId printerId, android.print.IPrintSpoolerCallbacks callbacks, int sequence) throws android.os.RemoteException
    {
    }
    /**
         * Clear all state from the custom printer icon cache.
         *
         * @param callbacks the callback to call once cache is cleared
         * @param sequence the sequence number of the call
         */
    @Override public void clearCustomPrinterIconCache(android.print.IPrintSpoolerCallbacks callbacks, int sequence) throws android.os.RemoteException
    {
    }
    @Override public void setPrintJobTag(android.print.PrintJobId printJobId, java.lang.String tag, android.print.IPrintSpoolerCallbacks callback, int sequence) throws android.os.RemoteException
    {
    }
    @Override public void writePrintJobData(android.os.ParcelFileDescriptor fd, android.print.PrintJobId printJobId) throws android.os.RemoteException
    {
    }
    @Override public void setClient(android.print.IPrintSpoolerClient client) throws android.os.RemoteException
    {
    }
    @Override public void setPrintJobCancelling(android.print.PrintJobId printJobId, boolean cancelling) throws android.os.RemoteException
    {
    }
    /**
         * Remove all approved print services that are not in the given set.
         *
         * @param servicesToKeep The names of the services to keep
         */
    @Override public void pruneApprovedPrintServices(java.util.List<android.content.ComponentName> servicesToKeep) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.print.IPrintSpooler
  {
    private static final java.lang.String DESCRIPTOR = "android.print.IPrintSpooler";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.print.IPrintSpooler interface,
     * generating a proxy if needed.
     */
    public static android.print.IPrintSpooler asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.print.IPrintSpooler))) {
        return ((android.print.IPrintSpooler)iin);
      }
      return new android.print.IPrintSpooler.Stub.Proxy(obj);
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
        case TRANSACTION_removeObsoletePrintJobs:
        {
          data.enforceInterface(descriptor);
          this.removeObsoletePrintJobs();
          return true;
        }
        case TRANSACTION_getPrintJobInfos:
        {
          data.enforceInterface(descriptor);
          android.print.IPrintSpoolerCallbacks _arg0;
          _arg0 = android.print.IPrintSpoolerCallbacks.Stub.asInterface(data.readStrongBinder());
          android.content.ComponentName _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          int _arg4;
          _arg4 = data.readInt();
          this.getPrintJobInfos(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        case TRANSACTION_getPrintJobInfo:
        {
          data.enforceInterface(descriptor);
          android.print.PrintJobId _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.print.PrintJobId.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.print.IPrintSpoolerCallbacks _arg1;
          _arg1 = android.print.IPrintSpoolerCallbacks.Stub.asInterface(data.readStrongBinder());
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          this.getPrintJobInfo(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_createPrintJob:
        {
          data.enforceInterface(descriptor);
          android.print.PrintJobInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.print.PrintJobInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.createPrintJob(_arg0);
          return true;
        }
        case TRANSACTION_setPrintJobState:
        {
          data.enforceInterface(descriptor);
          android.print.PrintJobId _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.print.PrintJobId.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.print.IPrintSpoolerCallbacks _arg3;
          _arg3 = android.print.IPrintSpoolerCallbacks.Stub.asInterface(data.readStrongBinder());
          int _arg4;
          _arg4 = data.readInt();
          this.setPrintJobState(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        case TRANSACTION_setProgress:
        {
          data.enforceInterface(descriptor);
          android.print.PrintJobId _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.print.PrintJobId.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          float _arg1;
          _arg1 = data.readFloat();
          this.setProgress(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_setStatus:
        {
          data.enforceInterface(descriptor);
          android.print.PrintJobId _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.print.PrintJobId.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.CharSequence _arg1;
          if (0!=data.readInt()) {
            _arg1 = android.text.TextUtils.CHAR_SEQUENCE_CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.setStatus(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_setStatusRes:
        {
          data.enforceInterface(descriptor);
          android.print.PrintJobId _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.print.PrintJobId.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          java.lang.CharSequence _arg2;
          if (0!=data.readInt()) {
            _arg2 = android.text.TextUtils.CHAR_SEQUENCE_CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.setStatusRes(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onCustomPrinterIconLoaded:
        {
          data.enforceInterface(descriptor);
          android.print.PrinterId _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.print.PrinterId.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.graphics.drawable.Icon _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.graphics.drawable.Icon.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.print.IPrintSpoolerCallbacks _arg2;
          _arg2 = android.print.IPrintSpoolerCallbacks.Stub.asInterface(data.readStrongBinder());
          int _arg3;
          _arg3 = data.readInt();
          this.onCustomPrinterIconLoaded(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_getCustomPrinterIcon:
        {
          data.enforceInterface(descriptor);
          android.print.PrinterId _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.print.PrinterId.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.print.IPrintSpoolerCallbacks _arg1;
          _arg1 = android.print.IPrintSpoolerCallbacks.Stub.asInterface(data.readStrongBinder());
          int _arg2;
          _arg2 = data.readInt();
          this.getCustomPrinterIcon(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_clearCustomPrinterIconCache:
        {
          data.enforceInterface(descriptor);
          android.print.IPrintSpoolerCallbacks _arg0;
          _arg0 = android.print.IPrintSpoolerCallbacks.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          this.clearCustomPrinterIconCache(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_setPrintJobTag:
        {
          data.enforceInterface(descriptor);
          android.print.PrintJobId _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.print.PrintJobId.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.print.IPrintSpoolerCallbacks _arg2;
          _arg2 = android.print.IPrintSpoolerCallbacks.Stub.asInterface(data.readStrongBinder());
          int _arg3;
          _arg3 = data.readInt();
          this.setPrintJobTag(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_writePrintJobData:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelFileDescriptor _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.print.PrintJobId _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.print.PrintJobId.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.writePrintJobData(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_setClient:
        {
          data.enforceInterface(descriptor);
          android.print.IPrintSpoolerClient _arg0;
          _arg0 = android.print.IPrintSpoolerClient.Stub.asInterface(data.readStrongBinder());
          this.setClient(_arg0);
          return true;
        }
        case TRANSACTION_setPrintJobCancelling:
        {
          data.enforceInterface(descriptor);
          android.print.PrintJobId _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.print.PrintJobId.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setPrintJobCancelling(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_pruneApprovedPrintServices:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.content.ComponentName> _arg0;
          _arg0 = data.createTypedArrayList(android.content.ComponentName.CREATOR);
          this.pruneApprovedPrintServices(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.print.IPrintSpooler
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
      @Override public void removeObsoletePrintJobs() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeObsoletePrintJobs, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeObsoletePrintJobs();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void getPrintJobInfos(android.print.IPrintSpoolerCallbacks callback, android.content.ComponentName componentName, int state, int appId, int sequence) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          if ((componentName!=null)) {
            _data.writeInt(1);
            componentName.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(state);
          _data.writeInt(appId);
          _data.writeInt(sequence);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPrintJobInfos, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getPrintJobInfos(callback, componentName, state, appId, sequence);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void getPrintJobInfo(android.print.PrintJobId printJobId, android.print.IPrintSpoolerCallbacks callback, int appId, int sequence) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((printJobId!=null)) {
            _data.writeInt(1);
            printJobId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeInt(appId);
          _data.writeInt(sequence);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPrintJobInfo, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getPrintJobInfo(printJobId, callback, appId, sequence);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void createPrintJob(android.print.PrintJobInfo printJob) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_createPrintJob, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().createPrintJob(printJob);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setPrintJobState(android.print.PrintJobId printJobId, int status, java.lang.String stateReason, android.print.IPrintSpoolerCallbacks callback, int sequence) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((printJobId!=null)) {
            _data.writeInt(1);
            printJobId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(status);
          _data.writeString(stateReason);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeInt(sequence);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setPrintJobState, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setPrintJobState(printJobId, status, stateReason, callback, sequence);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Set the progress of this print job
           *
           * @param printJobId The print job to update
           * @param progress The new progress
           */
      @Override public void setProgress(android.print.PrintJobId printJobId, float progress) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((printJobId!=null)) {
            _data.writeInt(1);
            printJobId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeFloat(progress);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setProgress, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setProgress(printJobId, progress);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Set the status of this print job
           *
           * @param printJobId The print job to update
           * @param status The new status, can be null
           */
      @Override public void setStatus(android.print.PrintJobId printJobId, java.lang.CharSequence status) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((printJobId!=null)) {
            _data.writeInt(1);
            printJobId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if (status!=null) {
            _data.writeInt(1);
            android.text.TextUtils.writeToParcel(status, _data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setStatus, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setStatus(printJobId, status);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Set the status of this print job
           *
           * @param printJobId The print job to update
           * @param status The new status as a string resource
           * @param appPackageName App package name the resource belongs to
           */
      @Override public void setStatusRes(android.print.PrintJobId printJobId, int status, java.lang.CharSequence appPackageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((printJobId!=null)) {
            _data.writeInt(1);
            printJobId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(status);
          if (appPackageName!=null) {
            _data.writeInt(1);
            android.text.TextUtils.writeToParcel(appPackageName, _data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setStatusRes, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setStatusRes(printJobId, status, appPackageName);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Handle that a custom icon for a printer was loaded.
           *
           * @param printerId the id of the printer the icon belongs to
           * @param icon the icon that was loaded
           * @param callbacks the callback to call once icon is stored in case
           * @param sequence the sequence number of the call
           * @see android.print.PrinterInfo.Builder#setHasCustomPrinterIcon()
           */
      @Override public void onCustomPrinterIconLoaded(android.print.PrinterId printerId, android.graphics.drawable.Icon icon, android.print.IPrintSpoolerCallbacks callbacks, int sequence) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((printerId!=null)) {
            _data.writeInt(1);
            printerId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((icon!=null)) {
            _data.writeInt(1);
            icon.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callbacks!=null))?(callbacks.asBinder()):(null)));
          _data.writeInt(sequence);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onCustomPrinterIconLoaded, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onCustomPrinterIconLoaded(printerId, icon, callbacks, sequence);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Get the custom icon for a printer. If the icon is not cached, the icon is
           * requested asynchronously. Once it is available the printer is updated.
           *
           * @param printerId the id of the printer the icon should be loaded for
           * @param callbacks the callback to call once icon is retrieved
           * @param sequence the sequence number of the call
           * @see android.print.PrinterInfo.Builder#setHasCustomPrinterIcon()
           */
      @Override public void getCustomPrinterIcon(android.print.PrinterId printerId, android.print.IPrintSpoolerCallbacks callbacks, int sequence) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((printerId!=null)) {
            _data.writeInt(1);
            printerId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callbacks!=null))?(callbacks.asBinder()):(null)));
          _data.writeInt(sequence);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCustomPrinterIcon, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getCustomPrinterIcon(printerId, callbacks, sequence);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Clear all state from the custom printer icon cache.
           *
           * @param callbacks the callback to call once cache is cleared
           * @param sequence the sequence number of the call
           */
      @Override public void clearCustomPrinterIconCache(android.print.IPrintSpoolerCallbacks callbacks, int sequence) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callbacks!=null))?(callbacks.asBinder()):(null)));
          _data.writeInt(sequence);
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearCustomPrinterIconCache, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().clearCustomPrinterIconCache(callbacks, sequence);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setPrintJobTag(android.print.PrintJobId printJobId, java.lang.String tag, android.print.IPrintSpoolerCallbacks callback, int sequence) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((printJobId!=null)) {
            _data.writeInt(1);
            printJobId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(tag);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeInt(sequence);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setPrintJobTag, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setPrintJobTag(printJobId, tag, callback, sequence);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void writePrintJobData(android.os.ParcelFileDescriptor fd, android.print.PrintJobId printJobId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((fd!=null)) {
            _data.writeInt(1);
            fd.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((printJobId!=null)) {
            _data.writeInt(1);
            printJobId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_writePrintJobData, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().writePrintJobData(fd, printJobId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setClient(android.print.IPrintSpoolerClient client) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((client!=null))?(client.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setClient, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setClient(client);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setPrintJobCancelling(android.print.PrintJobId printJobId, boolean cancelling) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((printJobId!=null)) {
            _data.writeInt(1);
            printJobId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((cancelling)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setPrintJobCancelling, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setPrintJobCancelling(printJobId, cancelling);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Remove all approved print services that are not in the given set.
           *
           * @param servicesToKeep The names of the services to keep
           */
      @Override public void pruneApprovedPrintServices(java.util.List<android.content.ComponentName> servicesToKeep) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedList(servicesToKeep);
          boolean _status = mRemote.transact(Stub.TRANSACTION_pruneApprovedPrintServices, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().pruneApprovedPrintServices(servicesToKeep);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.print.IPrintSpooler sDefaultImpl;
    }
    static final int TRANSACTION_removeObsoletePrintJobs = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getPrintJobInfos = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getPrintJobInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_createPrintJob = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_setPrintJobState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_setProgress = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_setStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_setStatusRes = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_onCustomPrinterIconLoaded = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_getCustomPrinterIcon = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_clearCustomPrinterIconCache = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_setPrintJobTag = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_writePrintJobData = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_setClient = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_setPrintJobCancelling = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_pruneApprovedPrintServices = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    public static boolean setDefaultImpl(android.print.IPrintSpooler impl) {
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
    public static android.print.IPrintSpooler getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void removeObsoletePrintJobs() throws android.os.RemoteException;
  public void getPrintJobInfos(android.print.IPrintSpoolerCallbacks callback, android.content.ComponentName componentName, int state, int appId, int sequence) throws android.os.RemoteException;
  public void getPrintJobInfo(android.print.PrintJobId printJobId, android.print.IPrintSpoolerCallbacks callback, int appId, int sequence) throws android.os.RemoteException;
  public void createPrintJob(android.print.PrintJobInfo printJob) throws android.os.RemoteException;
  public void setPrintJobState(android.print.PrintJobId printJobId, int status, java.lang.String stateReason, android.print.IPrintSpoolerCallbacks callback, int sequence) throws android.os.RemoteException;
  /**
       * Set the progress of this print job
       *
       * @param printJobId The print job to update
       * @param progress The new progress
       */
  public void setProgress(android.print.PrintJobId printJobId, float progress) throws android.os.RemoteException;
  /**
       * Set the status of this print job
       *
       * @param printJobId The print job to update
       * @param status The new status, can be null
       */
  public void setStatus(android.print.PrintJobId printJobId, java.lang.CharSequence status) throws android.os.RemoteException;
  /**
       * Set the status of this print job
       *
       * @param printJobId The print job to update
       * @param status The new status as a string resource
       * @param appPackageName App package name the resource belongs to
       */
  public void setStatusRes(android.print.PrintJobId printJobId, int status, java.lang.CharSequence appPackageName) throws android.os.RemoteException;
  /**
       * Handle that a custom icon for a printer was loaded.
       *
       * @param printerId the id of the printer the icon belongs to
       * @param icon the icon that was loaded
       * @param callbacks the callback to call once icon is stored in case
       * @param sequence the sequence number of the call
       * @see android.print.PrinterInfo.Builder#setHasCustomPrinterIcon()
       */
  public void onCustomPrinterIconLoaded(android.print.PrinterId printerId, android.graphics.drawable.Icon icon, android.print.IPrintSpoolerCallbacks callbacks, int sequence) throws android.os.RemoteException;
  /**
       * Get the custom icon for a printer. If the icon is not cached, the icon is
       * requested asynchronously. Once it is available the printer is updated.
       *
       * @param printerId the id of the printer the icon should be loaded for
       * @param callbacks the callback to call once icon is retrieved
       * @param sequence the sequence number of the call
       * @see android.print.PrinterInfo.Builder#setHasCustomPrinterIcon()
       */
  public void getCustomPrinterIcon(android.print.PrinterId printerId, android.print.IPrintSpoolerCallbacks callbacks, int sequence) throws android.os.RemoteException;
  /**
       * Clear all state from the custom printer icon cache.
       *
       * @param callbacks the callback to call once cache is cleared
       * @param sequence the sequence number of the call
       */
  public void clearCustomPrinterIconCache(android.print.IPrintSpoolerCallbacks callbacks, int sequence) throws android.os.RemoteException;
  public void setPrintJobTag(android.print.PrintJobId printJobId, java.lang.String tag, android.print.IPrintSpoolerCallbacks callback, int sequence) throws android.os.RemoteException;
  public void writePrintJobData(android.os.ParcelFileDescriptor fd, android.print.PrintJobId printJobId) throws android.os.RemoteException;
  public void setClient(android.print.IPrintSpoolerClient client) throws android.os.RemoteException;
  public void setPrintJobCancelling(android.print.PrintJobId printJobId, boolean cancelling) throws android.os.RemoteException;
  /**
       * Remove all approved print services that are not in the given set.
       *
       * @param servicesToKeep The names of the services to keep
       */
  public void pruneApprovedPrintServices(java.util.List<android.content.ComponentName> servicesToKeep) throws android.os.RemoteException;
}
