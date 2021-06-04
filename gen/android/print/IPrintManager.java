/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.print;
/**
 * Interface for communication with the core print manager service.
 *
 * @hide
 */
public interface IPrintManager extends android.os.IInterface
{
  /** Default implementation for IPrintManager. */
  public static class Default implements android.print.IPrintManager
  {
    @Override public java.util.List<android.print.PrintJobInfo> getPrintJobInfos(int appId, int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.print.PrintJobInfo getPrintJobInfo(android.print.PrintJobId printJobId, int appId, int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.os.Bundle print(java.lang.String printJobName, android.print.IPrintDocumentAdapter printAdapter, android.print.PrintAttributes attributes, java.lang.String packageName, int appId, int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void cancelPrintJob(android.print.PrintJobId printJobId, int appId, int userId) throws android.os.RemoteException
    {
    }
    @Override public void restartPrintJob(android.print.PrintJobId printJobId, int appId, int userId) throws android.os.RemoteException
    {
    }
    @Override public void addPrintJobStateChangeListener(android.print.IPrintJobStateChangeListener listener, int appId, int userId) throws android.os.RemoteException
    {
    }
    @Override public void removePrintJobStateChangeListener(android.print.IPrintJobStateChangeListener listener, int userId) throws android.os.RemoteException
    {
    }
    /**
         * Listen for changes to the installed and enabled print services.
         *
         * @param listener the listener to add
         * @param userId the id of the user listening
         *
         * @see android.print.PrintManager#getPrintServices(int, String)
         */
    @Override public void addPrintServicesChangeListener(android.print.IPrintServicesChangeListener listener, int userId) throws android.os.RemoteException
    {
    }
    /**
         * Stop listening for changes to the installed and enabled print services.
         *
         * @param listener the listener to remove
         * @param userId the id of the user requesting the removal
         *
         * @see android.print.PrintManager#getPrintServices(int, String)
         */
    @Override public void removePrintServicesChangeListener(android.print.IPrintServicesChangeListener listener, int userId) throws android.os.RemoteException
    {
    }
    /**
         * Get the print services.
         *
         * @param selectionFlags flags selecting which services to get
         * @param userId the id of the user requesting the services
         *
         * @return the list of selected print services.
         */
    @Override public java.util.List<android.printservice.PrintServiceInfo> getPrintServices(int selectionFlags, int userId) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Enable or disable a print service.
         *
         * @param service The service to enabled or disable
         * @param isEnabled whether the service should be enabled or disabled
         * @param userId the id of the user requesting the services
         */
    @Override public void setPrintServiceEnabled(android.content.ComponentName service, boolean isEnabled, int userId) throws android.os.RemoteException
    {
    }
    /**
         * Listen for changes to the print service recommendations.
         *
         * @param listener the listener to add
         * @param userId the id of the user listening
         *
         * @see android.print.PrintManager#getPrintServiceRecommendations
         */
    @Override public void addPrintServiceRecommendationsChangeListener(android.printservice.recommendation.IRecommendationsChangeListener listener, int userId) throws android.os.RemoteException
    {
    }
    /**
         * Stop listening for changes to the print service recommendations.
         *
         * @param listener the listener to remove
         * @param userId the id of the user requesting the removal
         *
         * @see android.print.PrintManager#getPrintServiceRecommendations
         */
    @Override public void removePrintServiceRecommendationsChangeListener(android.printservice.recommendation.IRecommendationsChangeListener listener, int userId) throws android.os.RemoteException
    {
    }
    /**
         * Get the print service recommendations.
         *
         * @param userId the id of the user requesting the recommendations
         *
         * @return the list of selected print services.
         */
    @Override public java.util.List<android.printservice.recommendation.RecommendationInfo> getPrintServiceRecommendations(int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void createPrinterDiscoverySession(android.print.IPrinterDiscoveryObserver observer, int userId) throws android.os.RemoteException
    {
    }
    @Override public void startPrinterDiscovery(android.print.IPrinterDiscoveryObserver observer, java.util.List<android.print.PrinterId> priorityList, int userId) throws android.os.RemoteException
    {
    }
    @Override public void stopPrinterDiscovery(android.print.IPrinterDiscoveryObserver observer, int userId) throws android.os.RemoteException
    {
    }
    @Override public void validatePrinters(java.util.List<android.print.PrinterId> printerIds, int userId) throws android.os.RemoteException
    {
    }
    @Override public void startPrinterStateTracking(android.print.PrinterId printerId, int userId) throws android.os.RemoteException
    {
    }
    /**
         * Get the custom icon for a printer. If the icon is not cached, the icon is
         * requested asynchronously. Once it is available the printer is updated.
         *
         * @param printerId the id of the printer the icon should be loaded for
         * @param userId the id of the user requesting the printer
         * @return the custom icon to be used for the printer or null if the icon is
         *         not yet available
         * @see android.print.PrinterInfo.Builder#setHasCustomPrinterIcon()
         */
    @Override public android.graphics.drawable.Icon getCustomPrinterIcon(android.print.PrinterId printerId, int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void stopPrinterStateTracking(android.print.PrinterId printerId, int userId) throws android.os.RemoteException
    {
    }
    @Override public void destroyPrinterDiscoverySession(android.print.IPrinterDiscoveryObserver observer, int userId) throws android.os.RemoteException
    {
    }
    /**
         * Check if the system will bind to print services in intant app.
         *
         * @param userId the Id of the user the behavior should be checked for
         *
         * @return {@code true} iff the system will bind to print services in instant apps.
         */
    @Override public boolean getBindInstantServiceAllowed(int userId) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Set if the system will bind to print services in intant app.
         *
         * @param userId the Id of the user the behavior should be changed for
         * @param allows iff {@code true} the system will bind to print services in instant apps
         */
    @Override public void setBindInstantServiceAllowed(int userId, boolean allowed) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.print.IPrintManager
  {
    private static final java.lang.String DESCRIPTOR = "android.print.IPrintManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.print.IPrintManager interface,
     * generating a proxy if needed.
     */
    public static android.print.IPrintManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.print.IPrintManager))) {
        return ((android.print.IPrintManager)iin);
      }
      return new android.print.IPrintManager.Stub.Proxy(obj);
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
        case TRANSACTION_getPrintJobInfos:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.util.List<android.print.PrintJobInfo> _result = this.getPrintJobInfos(_arg0, _arg1);
          reply.writeNoException();
          reply.writeTypedList(_result);
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
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          android.print.PrintJobInfo _result = this.getPrintJobInfo(_arg0, _arg1, _arg2);
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_print:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.print.IPrintDocumentAdapter _arg1;
          _arg1 = android.print.IPrintDocumentAdapter.Stub.asInterface(data.readStrongBinder());
          android.print.PrintAttributes _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.print.PrintAttributes.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          java.lang.String _arg3;
          _arg3 = data.readString();
          int _arg4;
          _arg4 = data.readInt();
          int _arg5;
          _arg5 = data.readInt();
          android.os.Bundle _result = this.print(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_cancelPrintJob:
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
          int _arg2;
          _arg2 = data.readInt();
          this.cancelPrintJob(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_restartPrintJob:
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
          int _arg2;
          _arg2 = data.readInt();
          this.restartPrintJob(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addPrintJobStateChangeListener:
        {
          data.enforceInterface(descriptor);
          android.print.IPrintJobStateChangeListener _arg0;
          _arg0 = android.print.IPrintJobStateChangeListener.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.addPrintJobStateChangeListener(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removePrintJobStateChangeListener:
        {
          data.enforceInterface(descriptor);
          android.print.IPrintJobStateChangeListener _arg0;
          _arg0 = android.print.IPrintJobStateChangeListener.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          this.removePrintJobStateChangeListener(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addPrintServicesChangeListener:
        {
          data.enforceInterface(descriptor);
          android.print.IPrintServicesChangeListener _arg0;
          _arg0 = android.print.IPrintServicesChangeListener.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          this.addPrintServicesChangeListener(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removePrintServicesChangeListener:
        {
          data.enforceInterface(descriptor);
          android.print.IPrintServicesChangeListener _arg0;
          _arg0 = android.print.IPrintServicesChangeListener.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          this.removePrintServicesChangeListener(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getPrintServices:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.util.List<android.printservice.PrintServiceInfo> _result = this.getPrintServices(_arg0, _arg1);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_setPrintServiceEnabled:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          int _arg2;
          _arg2 = data.readInt();
          this.setPrintServiceEnabled(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addPrintServiceRecommendationsChangeListener:
        {
          data.enforceInterface(descriptor);
          android.printservice.recommendation.IRecommendationsChangeListener _arg0;
          _arg0 = android.printservice.recommendation.IRecommendationsChangeListener.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          this.addPrintServiceRecommendationsChangeListener(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removePrintServiceRecommendationsChangeListener:
        {
          data.enforceInterface(descriptor);
          android.printservice.recommendation.IRecommendationsChangeListener _arg0;
          _arg0 = android.printservice.recommendation.IRecommendationsChangeListener.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          this.removePrintServiceRecommendationsChangeListener(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getPrintServiceRecommendations:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.util.List<android.printservice.recommendation.RecommendationInfo> _result = this.getPrintServiceRecommendations(_arg0);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_createPrinterDiscoverySession:
        {
          data.enforceInterface(descriptor);
          android.print.IPrinterDiscoveryObserver _arg0;
          _arg0 = android.print.IPrinterDiscoveryObserver.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          this.createPrinterDiscoverySession(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startPrinterDiscovery:
        {
          data.enforceInterface(descriptor);
          android.print.IPrinterDiscoveryObserver _arg0;
          _arg0 = android.print.IPrinterDiscoveryObserver.Stub.asInterface(data.readStrongBinder());
          java.util.List<android.print.PrinterId> _arg1;
          _arg1 = data.createTypedArrayList(android.print.PrinterId.CREATOR);
          int _arg2;
          _arg2 = data.readInt();
          this.startPrinterDiscovery(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_stopPrinterDiscovery:
        {
          data.enforceInterface(descriptor);
          android.print.IPrinterDiscoveryObserver _arg0;
          _arg0 = android.print.IPrinterDiscoveryObserver.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          this.stopPrinterDiscovery(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_validatePrinters:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.print.PrinterId> _arg0;
          _arg0 = data.createTypedArrayList(android.print.PrinterId.CREATOR);
          int _arg1;
          _arg1 = data.readInt();
          this.validatePrinters(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startPrinterStateTracking:
        {
          data.enforceInterface(descriptor);
          android.print.PrinterId _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.print.PrinterId.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          this.startPrinterStateTracking(_arg0, _arg1);
          reply.writeNoException();
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
          int _arg1;
          _arg1 = data.readInt();
          android.graphics.drawable.Icon _result = this.getCustomPrinterIcon(_arg0, _arg1);
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_stopPrinterStateTracking:
        {
          data.enforceInterface(descriptor);
          android.print.PrinterId _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.print.PrinterId.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          this.stopPrinterStateTracking(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_destroyPrinterDiscoverySession:
        {
          data.enforceInterface(descriptor);
          android.print.IPrinterDiscoveryObserver _arg0;
          _arg0 = android.print.IPrinterDiscoveryObserver.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          this.destroyPrinterDiscoverySession(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getBindInstantServiceAllowed:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.getBindInstantServiceAllowed(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setBindInstantServiceAllowed:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setBindInstantServiceAllowed(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.print.IPrintManager
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
      @Override public java.util.List<android.print.PrintJobInfo> getPrintJobInfos(int appId, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.print.PrintJobInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(appId);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPrintJobInfos, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPrintJobInfos(appId, userId);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.print.PrintJobInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.print.PrintJobInfo getPrintJobInfo(android.print.PrintJobId printJobId, int appId, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.print.PrintJobInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((printJobId!=null)) {
            _data.writeInt(1);
            printJobId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(appId);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPrintJobInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPrintJobInfo(printJobId, appId, userId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.print.PrintJobInfo.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.os.Bundle print(java.lang.String printJobName, android.print.IPrintDocumentAdapter printAdapter, android.print.PrintAttributes attributes, java.lang.String packageName, int appId, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.Bundle _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(printJobName);
          _data.writeStrongBinder((((printAdapter!=null))?(printAdapter.asBinder()):(null)));
          if ((attributes!=null)) {
            _data.writeInt(1);
            attributes.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(packageName);
          _data.writeInt(appId);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_print, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().print(printJobName, printAdapter, attributes, packageName, appId, userId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.os.Bundle.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void cancelPrintJob(android.print.PrintJobId printJobId, int appId, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((printJobId!=null)) {
            _data.writeInt(1);
            printJobId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(appId);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_cancelPrintJob, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cancelPrintJob(printJobId, appId, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void restartPrintJob(android.print.PrintJobId printJobId, int appId, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((printJobId!=null)) {
            _data.writeInt(1);
            printJobId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(appId);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_restartPrintJob, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().restartPrintJob(printJobId, appId, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void addPrintJobStateChangeListener(android.print.IPrintJobStateChangeListener listener, int appId, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          _data.writeInt(appId);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addPrintJobStateChangeListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addPrintJobStateChangeListener(listener, appId, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removePrintJobStateChangeListener(android.print.IPrintJobStateChangeListener listener, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removePrintJobStateChangeListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removePrintJobStateChangeListener(listener, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Listen for changes to the installed and enabled print services.
           *
           * @param listener the listener to add
           * @param userId the id of the user listening
           *
           * @see android.print.PrintManager#getPrintServices(int, String)
           */
      @Override public void addPrintServicesChangeListener(android.print.IPrintServicesChangeListener listener, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addPrintServicesChangeListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addPrintServicesChangeListener(listener, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Stop listening for changes to the installed and enabled print services.
           *
           * @param listener the listener to remove
           * @param userId the id of the user requesting the removal
           *
           * @see android.print.PrintManager#getPrintServices(int, String)
           */
      @Override public void removePrintServicesChangeListener(android.print.IPrintServicesChangeListener listener, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removePrintServicesChangeListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removePrintServicesChangeListener(listener, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Get the print services.
           *
           * @param selectionFlags flags selecting which services to get
           * @param userId the id of the user requesting the services
           *
           * @return the list of selected print services.
           */
      @Override public java.util.List<android.printservice.PrintServiceInfo> getPrintServices(int selectionFlags, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.printservice.PrintServiceInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(selectionFlags);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPrintServices, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPrintServices(selectionFlags, userId);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.printservice.PrintServiceInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Enable or disable a print service.
           *
           * @param service The service to enabled or disable
           * @param isEnabled whether the service should be enabled or disabled
           * @param userId the id of the user requesting the services
           */
      @Override public void setPrintServiceEnabled(android.content.ComponentName service, boolean isEnabled, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((service!=null)) {
            _data.writeInt(1);
            service.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((isEnabled)?(1):(0)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setPrintServiceEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setPrintServiceEnabled(service, isEnabled, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Listen for changes to the print service recommendations.
           *
           * @param listener the listener to add
           * @param userId the id of the user listening
           *
           * @see android.print.PrintManager#getPrintServiceRecommendations
           */
      @Override public void addPrintServiceRecommendationsChangeListener(android.printservice.recommendation.IRecommendationsChangeListener listener, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addPrintServiceRecommendationsChangeListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addPrintServiceRecommendationsChangeListener(listener, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Stop listening for changes to the print service recommendations.
           *
           * @param listener the listener to remove
           * @param userId the id of the user requesting the removal
           *
           * @see android.print.PrintManager#getPrintServiceRecommendations
           */
      @Override public void removePrintServiceRecommendationsChangeListener(android.printservice.recommendation.IRecommendationsChangeListener listener, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removePrintServiceRecommendationsChangeListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removePrintServiceRecommendationsChangeListener(listener, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Get the print service recommendations.
           *
           * @param userId the id of the user requesting the recommendations
           *
           * @return the list of selected print services.
           */
      @Override public java.util.List<android.printservice.recommendation.RecommendationInfo> getPrintServiceRecommendations(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.printservice.recommendation.RecommendationInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPrintServiceRecommendations, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPrintServiceRecommendations(userId);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.printservice.recommendation.RecommendationInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void createPrinterDiscoverySession(android.print.IPrinterDiscoveryObserver observer, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((observer!=null))?(observer.asBinder()):(null)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_createPrinterDiscoverySession, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().createPrinterDiscoverySession(observer, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void startPrinterDiscovery(android.print.IPrinterDiscoveryObserver observer, java.util.List<android.print.PrinterId> priorityList, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((observer!=null))?(observer.asBinder()):(null)));
          _data.writeTypedList(priorityList);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startPrinterDiscovery, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startPrinterDiscovery(observer, priorityList, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void stopPrinterDiscovery(android.print.IPrinterDiscoveryObserver observer, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((observer!=null))?(observer.asBinder()):(null)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopPrinterDiscovery, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopPrinterDiscovery(observer, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void validatePrinters(java.util.List<android.print.PrinterId> printerIds, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedList(printerIds);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_validatePrinters, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().validatePrinters(printerIds, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void startPrinterStateTracking(android.print.PrinterId printerId, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((printerId!=null)) {
            _data.writeInt(1);
            printerId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startPrinterStateTracking, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startPrinterStateTracking(printerId, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Get the custom icon for a printer. If the icon is not cached, the icon is
           * requested asynchronously. Once it is available the printer is updated.
           *
           * @param printerId the id of the printer the icon should be loaded for
           * @param userId the id of the user requesting the printer
           * @return the custom icon to be used for the printer or null if the icon is
           *         not yet available
           * @see android.print.PrinterInfo.Builder#setHasCustomPrinterIcon()
           */
      @Override public android.graphics.drawable.Icon getCustomPrinterIcon(android.print.PrinterId printerId, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.graphics.drawable.Icon _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((printerId!=null)) {
            _data.writeInt(1);
            printerId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCustomPrinterIcon, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCustomPrinterIcon(printerId, userId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.graphics.drawable.Icon.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void stopPrinterStateTracking(android.print.PrinterId printerId, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((printerId!=null)) {
            _data.writeInt(1);
            printerId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopPrinterStateTracking, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopPrinterStateTracking(printerId, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void destroyPrinterDiscoverySession(android.print.IPrinterDiscoveryObserver observer, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((observer!=null))?(observer.asBinder()):(null)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_destroyPrinterDiscoverySession, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().destroyPrinterDiscoverySession(observer, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Check if the system will bind to print services in intant app.
           *
           * @param userId the Id of the user the behavior should be checked for
           *
           * @return {@code true} iff the system will bind to print services in instant apps.
           */
      @Override public boolean getBindInstantServiceAllowed(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getBindInstantServiceAllowed, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getBindInstantServiceAllowed(userId);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Set if the system will bind to print services in intant app.
           *
           * @param userId the Id of the user the behavior should be changed for
           * @param allows iff {@code true} the system will bind to print services in instant apps
           */
      @Override public void setBindInstantServiceAllowed(int userId, boolean allowed) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeInt(((allowed)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setBindInstantServiceAllowed, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setBindInstantServiceAllowed(userId, allowed);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.print.IPrintManager sDefaultImpl;
    }
    static final int TRANSACTION_getPrintJobInfos = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getPrintJobInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_print = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_cancelPrintJob = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_restartPrintJob = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_addPrintJobStateChangeListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_removePrintJobStateChangeListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_addPrintServicesChangeListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_removePrintServicesChangeListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_getPrintServices = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_setPrintServiceEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_addPrintServiceRecommendationsChangeListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_removePrintServiceRecommendationsChangeListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_getPrintServiceRecommendations = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_createPrinterDiscoverySession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_startPrinterDiscovery = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_stopPrinterDiscovery = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_validatePrinters = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_startPrinterStateTracking = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_getCustomPrinterIcon = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_stopPrinterStateTracking = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_destroyPrinterDiscoverySession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_getBindInstantServiceAllowed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_setBindInstantServiceAllowed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    public static boolean setDefaultImpl(android.print.IPrintManager impl) {
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
    public static android.print.IPrintManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public java.util.List<android.print.PrintJobInfo> getPrintJobInfos(int appId, int userId) throws android.os.RemoteException;
  public android.print.PrintJobInfo getPrintJobInfo(android.print.PrintJobId printJobId, int appId, int userId) throws android.os.RemoteException;
  public android.os.Bundle print(java.lang.String printJobName, android.print.IPrintDocumentAdapter printAdapter, android.print.PrintAttributes attributes, java.lang.String packageName, int appId, int userId) throws android.os.RemoteException;
  public void cancelPrintJob(android.print.PrintJobId printJobId, int appId, int userId) throws android.os.RemoteException;
  public void restartPrintJob(android.print.PrintJobId printJobId, int appId, int userId) throws android.os.RemoteException;
  public void addPrintJobStateChangeListener(android.print.IPrintJobStateChangeListener listener, int appId, int userId) throws android.os.RemoteException;
  public void removePrintJobStateChangeListener(android.print.IPrintJobStateChangeListener listener, int userId) throws android.os.RemoteException;
  /**
       * Listen for changes to the installed and enabled print services.
       *
       * @param listener the listener to add
       * @param userId the id of the user listening
       *
       * @see android.print.PrintManager#getPrintServices(int, String)
       */
  public void addPrintServicesChangeListener(android.print.IPrintServicesChangeListener listener, int userId) throws android.os.RemoteException;
  /**
       * Stop listening for changes to the installed and enabled print services.
       *
       * @param listener the listener to remove
       * @param userId the id of the user requesting the removal
       *
       * @see android.print.PrintManager#getPrintServices(int, String)
       */
  public void removePrintServicesChangeListener(android.print.IPrintServicesChangeListener listener, int userId) throws android.os.RemoteException;
  /**
       * Get the print services.
       *
       * @param selectionFlags flags selecting which services to get
       * @param userId the id of the user requesting the services
       *
       * @return the list of selected print services.
       */
  public java.util.List<android.printservice.PrintServiceInfo> getPrintServices(int selectionFlags, int userId) throws android.os.RemoteException;
  /**
       * Enable or disable a print service.
       *
       * @param service The service to enabled or disable
       * @param isEnabled whether the service should be enabled or disabled
       * @param userId the id of the user requesting the services
       */
  public void setPrintServiceEnabled(android.content.ComponentName service, boolean isEnabled, int userId) throws android.os.RemoteException;
  /**
       * Listen for changes to the print service recommendations.
       *
       * @param listener the listener to add
       * @param userId the id of the user listening
       *
       * @see android.print.PrintManager#getPrintServiceRecommendations
       */
  public void addPrintServiceRecommendationsChangeListener(android.printservice.recommendation.IRecommendationsChangeListener listener, int userId) throws android.os.RemoteException;
  /**
       * Stop listening for changes to the print service recommendations.
       *
       * @param listener the listener to remove
       * @param userId the id of the user requesting the removal
       *
       * @see android.print.PrintManager#getPrintServiceRecommendations
       */
  public void removePrintServiceRecommendationsChangeListener(android.printservice.recommendation.IRecommendationsChangeListener listener, int userId) throws android.os.RemoteException;
  /**
       * Get the print service recommendations.
       *
       * @param userId the id of the user requesting the recommendations
       *
       * @return the list of selected print services.
       */
  public java.util.List<android.printservice.recommendation.RecommendationInfo> getPrintServiceRecommendations(int userId) throws android.os.RemoteException;
  public void createPrinterDiscoverySession(android.print.IPrinterDiscoveryObserver observer, int userId) throws android.os.RemoteException;
  public void startPrinterDiscovery(android.print.IPrinterDiscoveryObserver observer, java.util.List<android.print.PrinterId> priorityList, int userId) throws android.os.RemoteException;
  public void stopPrinterDiscovery(android.print.IPrinterDiscoveryObserver observer, int userId) throws android.os.RemoteException;
  public void validatePrinters(java.util.List<android.print.PrinterId> printerIds, int userId) throws android.os.RemoteException;
  public void startPrinterStateTracking(android.print.PrinterId printerId, int userId) throws android.os.RemoteException;
  /**
       * Get the custom icon for a printer. If the icon is not cached, the icon is
       * requested asynchronously. Once it is available the printer is updated.
       *
       * @param printerId the id of the printer the icon should be loaded for
       * @param userId the id of the user requesting the printer
       * @return the custom icon to be used for the printer or null if the icon is
       *         not yet available
       * @see android.print.PrinterInfo.Builder#setHasCustomPrinterIcon()
       */
  public android.graphics.drawable.Icon getCustomPrinterIcon(android.print.PrinterId printerId, int userId) throws android.os.RemoteException;
  public void stopPrinterStateTracking(android.print.PrinterId printerId, int userId) throws android.os.RemoteException;
  public void destroyPrinterDiscoverySession(android.print.IPrinterDiscoveryObserver observer, int userId) throws android.os.RemoteException;
  /**
       * Check if the system will bind to print services in intant app.
       *
       * @param userId the Id of the user the behavior should be checked for
       *
       * @return {@code true} iff the system will bind to print services in instant apps.
       */
  public boolean getBindInstantServiceAllowed(int userId) throws android.os.RemoteException;
  /**
       * Set if the system will bind to print services in intant app.
       *
       * @param userId the Id of the user the behavior should be changed for
       * @param allows iff {@code true} the system will bind to print services in instant apps
       */
  public void setBindInstantServiceAllowed(int userId, boolean allowed) throws android.os.RemoteException;
}
