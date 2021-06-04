/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.location;
/**
 * System private API for talking with the location service.
 *
 * @hide
 */
public interface ILocationManager extends android.os.IInterface
{
  /** Default implementation for ILocationManager. */
  public static class Default implements android.location.ILocationManager
  {
    @Override public void requestLocationUpdates(android.location.LocationRequest request, android.location.ILocationListener listener, android.app.PendingIntent intent, java.lang.String packageName) throws android.os.RemoteException
    {
    }
    @Override public void removeUpdates(android.location.ILocationListener listener, android.app.PendingIntent intent, java.lang.String packageName) throws android.os.RemoteException
    {
    }
    @Override public void requestGeofence(android.location.LocationRequest request, android.location.Geofence geofence, android.app.PendingIntent intent, java.lang.String packageName) throws android.os.RemoteException
    {
    }
    @Override public void removeGeofence(android.location.Geofence fence, android.app.PendingIntent intent, java.lang.String packageName) throws android.os.RemoteException
    {
    }
    @Override public android.location.Location getLastLocation(android.location.LocationRequest request, java.lang.String packageName) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean registerGnssStatusCallback(android.location.IGnssStatusListener callback, java.lang.String packageName) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void unregisterGnssStatusCallback(android.location.IGnssStatusListener callback) throws android.os.RemoteException
    {
    }
    @Override public boolean geocoderIsPresent() throws android.os.RemoteException
    {
      return false;
    }
    @Override public java.lang.String getFromLocation(double latitude, double longitude, int maxResults, android.location.GeocoderParams params, java.util.List<android.location.Address> addrs) throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.String getFromLocationName(java.lang.String locationName, double lowerLeftLatitude, double lowerLeftLongitude, double upperRightLatitude, double upperRightLongitude, int maxResults, android.location.GeocoderParams params, java.util.List<android.location.Address> addrs) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean sendNiResponse(int notifId, int userResponse) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean addGnssMeasurementsListener(android.location.IGnssMeasurementsListener listener, java.lang.String packageName) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void injectGnssMeasurementCorrections(android.location.GnssMeasurementCorrections corrections, java.lang.String packageName) throws android.os.RemoteException
    {
    }
    @Override public long getGnssCapabilities(java.lang.String packageName) throws android.os.RemoteException
    {
      return 0L;
    }
    @Override public void removeGnssMeasurementsListener(android.location.IGnssMeasurementsListener listener) throws android.os.RemoteException
    {
    }
    @Override public boolean addGnssNavigationMessageListener(android.location.IGnssNavigationMessageListener listener, java.lang.String packageName) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void removeGnssNavigationMessageListener(android.location.IGnssNavigationMessageListener listener) throws android.os.RemoteException
    {
    }
    @Override public int getGnssYearOfHardware() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public java.lang.String getGnssHardwareModelName() throws android.os.RemoteException
    {
      return null;
    }
    @Override public int getGnssBatchSize(java.lang.String packageName) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public boolean addGnssBatchingCallback(android.location.IBatchedLocationCallback callback, java.lang.String packageName) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void removeGnssBatchingCallback() throws android.os.RemoteException
    {
    }
    @Override public boolean startGnssBatch(long periodNanos, boolean wakeOnFifoFull, java.lang.String packageName) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void flushGnssBatch(java.lang.String packageName) throws android.os.RemoteException
    {
    }
    @Override public boolean stopGnssBatch() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean injectLocation(android.location.Location location) throws android.os.RemoteException
    {
      return false;
    }
    @Override public java.util.List<java.lang.String> getAllProviders() throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.util.List<java.lang.String> getProviders(android.location.Criteria criteria, boolean enabledOnly) throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.String getBestProvider(android.location.Criteria criteria, boolean enabledOnly) throws android.os.RemoteException
    {
      return null;
    }
    @Override public com.android.internal.location.ProviderProperties getProviderProperties(java.lang.String provider) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean isProviderPackage(java.lang.String packageName) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void setExtraLocationControllerPackage(java.lang.String packageName) throws android.os.RemoteException
    {
    }
    @Override public java.lang.String getExtraLocationControllerPackage() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setExtraLocationControllerPackageEnabled(boolean enabled) throws android.os.RemoteException
    {
    }
    @Override public boolean isExtraLocationControllerPackageEnabled() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isProviderEnabledForUser(java.lang.String provider, int userId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isLocationEnabledForUser(int userId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void addTestProvider(java.lang.String name, com.android.internal.location.ProviderProperties properties, java.lang.String opPackageName) throws android.os.RemoteException
    {
    }
    @Override public void removeTestProvider(java.lang.String provider, java.lang.String opPackageName) throws android.os.RemoteException
    {
    }
    @Override public void setTestProviderLocation(java.lang.String provider, android.location.Location loc, java.lang.String opPackageName) throws android.os.RemoteException
    {
    }
    @Override public void setTestProviderEnabled(java.lang.String provider, boolean enabled, java.lang.String opPackageName) throws android.os.RemoteException
    {
    }
    @Override public java.util.List<android.location.LocationRequest> getTestProviderCurrentRequests(java.lang.String provider, java.lang.String opPackageName) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.location.LocationTime getGnssTimeMillis() throws android.os.RemoteException
    {
      return null;
    }
    // --- deprecated ---

    @Override public void setTestProviderStatus(java.lang.String provider, int status, android.os.Bundle extras, long updateTime, java.lang.String opPackageName) throws android.os.RemoteException
    {
    }
    @Override public boolean sendExtraCommand(java.lang.String provider, java.lang.String command, android.os.Bundle extras) throws android.os.RemoteException
    {
      return false;
    }
    // --- internal ---
    // for reporting callback completion

    @Override public void locationCallbackFinished(android.location.ILocationListener listener) throws android.os.RemoteException
    {
    }
    // used by gts tests to verify whitelists

    @Override public java.lang.String[] getBackgroundThrottlingWhitelist() throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.String[] getIgnoreSettingsWhitelist() throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.location.ILocationManager
  {
    private static final java.lang.String DESCRIPTOR = "android.location.ILocationManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.location.ILocationManager interface,
     * generating a proxy if needed.
     */
    public static android.location.ILocationManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.location.ILocationManager))) {
        return ((android.location.ILocationManager)iin);
      }
      return new android.location.ILocationManager.Stub.Proxy(obj);
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
        case TRANSACTION_requestLocationUpdates:
        {
          data.enforceInterface(descriptor);
          android.location.LocationRequest _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.location.LocationRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.location.ILocationListener _arg1;
          _arg1 = android.location.ILocationListener.Stub.asInterface(data.readStrongBinder());
          android.app.PendingIntent _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.app.PendingIntent.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.requestLocationUpdates(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeUpdates:
        {
          data.enforceInterface(descriptor);
          android.location.ILocationListener _arg0;
          _arg0 = android.location.ILocationListener.Stub.asInterface(data.readStrongBinder());
          android.app.PendingIntent _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.app.PendingIntent.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.removeUpdates(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_requestGeofence:
        {
          data.enforceInterface(descriptor);
          android.location.LocationRequest _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.location.LocationRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.location.Geofence _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.location.Geofence.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.app.PendingIntent _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.app.PendingIntent.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.requestGeofence(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeGeofence:
        {
          data.enforceInterface(descriptor);
          android.location.Geofence _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.location.Geofence.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.app.PendingIntent _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.app.PendingIntent.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.removeGeofence(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getLastLocation:
        {
          data.enforceInterface(descriptor);
          android.location.LocationRequest _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.location.LocationRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.location.Location _result = this.getLastLocation(_arg0, _arg1);
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
        case TRANSACTION_registerGnssStatusCallback:
        {
          data.enforceInterface(descriptor);
          android.location.IGnssStatusListener _arg0;
          _arg0 = android.location.IGnssStatusListener.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.registerGnssStatusCallback(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_unregisterGnssStatusCallback:
        {
          data.enforceInterface(descriptor);
          android.location.IGnssStatusListener _arg0;
          _arg0 = android.location.IGnssStatusListener.Stub.asInterface(data.readStrongBinder());
          this.unregisterGnssStatusCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_geocoderIsPresent:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.geocoderIsPresent();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getFromLocation:
        {
          data.enforceInterface(descriptor);
          double _arg0;
          _arg0 = data.readDouble();
          double _arg1;
          _arg1 = data.readDouble();
          int _arg2;
          _arg2 = data.readInt();
          android.location.GeocoderParams _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.location.GeocoderParams.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          java.util.List<android.location.Address> _arg4;
          _arg4 = new java.util.ArrayList<android.location.Address>();
          java.lang.String _result = this.getFromLocation(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          reply.writeString(_result);
          reply.writeTypedList(_arg4);
          return true;
        }
        case TRANSACTION_getFromLocationName:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          double _arg1;
          _arg1 = data.readDouble();
          double _arg2;
          _arg2 = data.readDouble();
          double _arg3;
          _arg3 = data.readDouble();
          double _arg4;
          _arg4 = data.readDouble();
          int _arg5;
          _arg5 = data.readInt();
          android.location.GeocoderParams _arg6;
          if ((0!=data.readInt())) {
            _arg6 = android.location.GeocoderParams.CREATOR.createFromParcel(data);
          }
          else {
            _arg6 = null;
          }
          java.util.List<android.location.Address> _arg7;
          _arg7 = new java.util.ArrayList<android.location.Address>();
          java.lang.String _result = this.getFromLocationName(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7);
          reply.writeNoException();
          reply.writeString(_result);
          reply.writeTypedList(_arg7);
          return true;
        }
        case TRANSACTION_sendNiResponse:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.sendNiResponse(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_addGnssMeasurementsListener:
        {
          data.enforceInterface(descriptor);
          android.location.IGnssMeasurementsListener _arg0;
          _arg0 = android.location.IGnssMeasurementsListener.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.addGnssMeasurementsListener(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_injectGnssMeasurementCorrections:
        {
          data.enforceInterface(descriptor);
          android.location.GnssMeasurementCorrections _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.location.GnssMeasurementCorrections.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.injectGnssMeasurementCorrections(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getGnssCapabilities:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          long _result = this.getGnssCapabilities(_arg0);
          reply.writeNoException();
          reply.writeLong(_result);
          return true;
        }
        case TRANSACTION_removeGnssMeasurementsListener:
        {
          data.enforceInterface(descriptor);
          android.location.IGnssMeasurementsListener _arg0;
          _arg0 = android.location.IGnssMeasurementsListener.Stub.asInterface(data.readStrongBinder());
          this.removeGnssMeasurementsListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addGnssNavigationMessageListener:
        {
          data.enforceInterface(descriptor);
          android.location.IGnssNavigationMessageListener _arg0;
          _arg0 = android.location.IGnssNavigationMessageListener.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.addGnssNavigationMessageListener(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_removeGnssNavigationMessageListener:
        {
          data.enforceInterface(descriptor);
          android.location.IGnssNavigationMessageListener _arg0;
          _arg0 = android.location.IGnssNavigationMessageListener.Stub.asInterface(data.readStrongBinder());
          this.removeGnssNavigationMessageListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getGnssYearOfHardware:
        {
          data.enforceInterface(descriptor);
          int _result = this.getGnssYearOfHardware();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getGnssHardwareModelName:
        {
          data.enforceInterface(descriptor);
          java.lang.String _result = this.getGnssHardwareModelName();
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getGnssBatchSize:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _result = this.getGnssBatchSize(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_addGnssBatchingCallback:
        {
          data.enforceInterface(descriptor);
          android.location.IBatchedLocationCallback _arg0;
          _arg0 = android.location.IBatchedLocationCallback.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.addGnssBatchingCallback(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_removeGnssBatchingCallback:
        {
          data.enforceInterface(descriptor);
          this.removeGnssBatchingCallback();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startGnssBatch:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          java.lang.String _arg2;
          _arg2 = data.readString();
          boolean _result = this.startGnssBatch(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_flushGnssBatch:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.flushGnssBatch(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_stopGnssBatch:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.stopGnssBatch();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_injectLocation:
        {
          data.enforceInterface(descriptor);
          android.location.Location _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.location.Location.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _result = this.injectLocation(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getAllProviders:
        {
          data.enforceInterface(descriptor);
          java.util.List<java.lang.String> _result = this.getAllProviders();
          reply.writeNoException();
          reply.writeStringList(_result);
          return true;
        }
        case TRANSACTION_getProviders:
        {
          data.enforceInterface(descriptor);
          android.location.Criteria _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.location.Criteria.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          java.util.List<java.lang.String> _result = this.getProviders(_arg0, _arg1);
          reply.writeNoException();
          reply.writeStringList(_result);
          return true;
        }
        case TRANSACTION_getBestProvider:
        {
          data.enforceInterface(descriptor);
          android.location.Criteria _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.location.Criteria.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          java.lang.String _result = this.getBestProvider(_arg0, _arg1);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getProviderProperties:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          com.android.internal.location.ProviderProperties _result = this.getProviderProperties(_arg0);
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
        case TRANSACTION_isProviderPackage:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _result = this.isProviderPackage(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setExtraLocationControllerPackage:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.setExtraLocationControllerPackage(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getExtraLocationControllerPackage:
        {
          data.enforceInterface(descriptor);
          java.lang.String _result = this.getExtraLocationControllerPackage();
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_setExtraLocationControllerPackageEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setExtraLocationControllerPackageEnabled(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isExtraLocationControllerPackageEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isExtraLocationControllerPackageEnabled();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isProviderEnabledForUser:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.isProviderEnabledForUser(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isLocationEnabledForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isLocationEnabledForUser(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_addTestProvider:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          com.android.internal.location.ProviderProperties _arg1;
          if ((0!=data.readInt())) {
            _arg1 = com.android.internal.location.ProviderProperties.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.addTestProvider(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeTestProvider:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.removeTestProvider(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setTestProviderLocation:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.location.Location _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.location.Location.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.setTestProviderLocation(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setTestProviderEnabled:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.setTestProviderEnabled(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getTestProviderCurrentRequests:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.util.List<android.location.LocationRequest> _result = this.getTestProviderCurrentRequests(_arg0, _arg1);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getGnssTimeMillis:
        {
          data.enforceInterface(descriptor);
          android.location.LocationTime _result = this.getGnssTimeMillis();
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
        case TRANSACTION_setTestProviderStatus:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          android.os.Bundle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          long _arg3;
          _arg3 = data.readLong();
          java.lang.String _arg4;
          _arg4 = data.readString();
          this.setTestProviderStatus(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_sendExtraCommand:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.os.Bundle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          boolean _result = this.sendExtraCommand(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          if ((_arg2!=null)) {
            reply.writeInt(1);
            _arg2.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_locationCallbackFinished:
        {
          data.enforceInterface(descriptor);
          android.location.ILocationListener _arg0;
          _arg0 = android.location.ILocationListener.Stub.asInterface(data.readStrongBinder());
          this.locationCallbackFinished(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getBackgroundThrottlingWhitelist:
        {
          data.enforceInterface(descriptor);
          java.lang.String[] _result = this.getBackgroundThrottlingWhitelist();
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_getIgnoreSettingsWhitelist:
        {
          data.enforceInterface(descriptor);
          java.lang.String[] _result = this.getIgnoreSettingsWhitelist();
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.location.ILocationManager
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
      @Override public void requestLocationUpdates(android.location.LocationRequest request, android.location.ILocationListener listener, android.app.PendingIntent intent, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((request!=null)) {
            _data.writeInt(1);
            request.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          if ((intent!=null)) {
            _data.writeInt(1);
            intent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestLocationUpdates, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestLocationUpdates(request, listener, intent, packageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeUpdates(android.location.ILocationListener listener, android.app.PendingIntent intent, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          if ((intent!=null)) {
            _data.writeInt(1);
            intent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeUpdates, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeUpdates(listener, intent, packageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void requestGeofence(android.location.LocationRequest request, android.location.Geofence geofence, android.app.PendingIntent intent, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((request!=null)) {
            _data.writeInt(1);
            request.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((geofence!=null)) {
            _data.writeInt(1);
            geofence.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((intent!=null)) {
            _data.writeInt(1);
            intent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestGeofence, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestGeofence(request, geofence, intent, packageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeGeofence(android.location.Geofence fence, android.app.PendingIntent intent, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((fence!=null)) {
            _data.writeInt(1);
            fence.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((intent!=null)) {
            _data.writeInt(1);
            intent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeGeofence, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeGeofence(fence, intent, packageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.location.Location getLastLocation(android.location.LocationRequest request, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.location.Location _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((request!=null)) {
            _data.writeInt(1);
            request.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getLastLocation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getLastLocation(request, packageName);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.location.Location.CREATOR.createFromParcel(_reply);
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
      @Override public boolean registerGnssStatusCallback(android.location.IGnssStatusListener callback, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerGnssStatusCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().registerGnssStatusCallback(callback, packageName);
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
      @Override public void unregisterGnssStatusCallback(android.location.IGnssStatusListener callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterGnssStatusCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterGnssStatusCallback(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean geocoderIsPresent() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_geocoderIsPresent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().geocoderIsPresent();
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
      @Override public java.lang.String getFromLocation(double latitude, double longitude, int maxResults, android.location.GeocoderParams params, java.util.List<android.location.Address> addrs) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeDouble(latitude);
          _data.writeDouble(longitude);
          _data.writeInt(maxResults);
          if ((params!=null)) {
            _data.writeInt(1);
            params.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getFromLocation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getFromLocation(latitude, longitude, maxResults, params, addrs);
          }
          _reply.readException();
          _result = _reply.readString();
          _reply.readTypedList(addrs, android.location.Address.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.lang.String getFromLocationName(java.lang.String locationName, double lowerLeftLatitude, double lowerLeftLongitude, double upperRightLatitude, double upperRightLongitude, int maxResults, android.location.GeocoderParams params, java.util.List<android.location.Address> addrs) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(locationName);
          _data.writeDouble(lowerLeftLatitude);
          _data.writeDouble(lowerLeftLongitude);
          _data.writeDouble(upperRightLatitude);
          _data.writeDouble(upperRightLongitude);
          _data.writeInt(maxResults);
          if ((params!=null)) {
            _data.writeInt(1);
            params.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getFromLocationName, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getFromLocationName(locationName, lowerLeftLatitude, lowerLeftLongitude, upperRightLatitude, upperRightLongitude, maxResults, params, addrs);
          }
          _reply.readException();
          _result = _reply.readString();
          _reply.readTypedList(addrs, android.location.Address.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public boolean sendNiResponse(int notifId, int userResponse) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(notifId);
          _data.writeInt(userResponse);
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendNiResponse, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().sendNiResponse(notifId, userResponse);
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
      @Override public boolean addGnssMeasurementsListener(android.location.IGnssMeasurementsListener listener, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addGnssMeasurementsListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().addGnssMeasurementsListener(listener, packageName);
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
      @Override public void injectGnssMeasurementCorrections(android.location.GnssMeasurementCorrections corrections, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((corrections!=null)) {
            _data.writeInt(1);
            corrections.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_injectGnssMeasurementCorrections, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().injectGnssMeasurementCorrections(corrections, packageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public long getGnssCapabilities(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getGnssCapabilities, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getGnssCapabilities(packageName);
          }
          _reply.readException();
          _result = _reply.readLong();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void removeGnssMeasurementsListener(android.location.IGnssMeasurementsListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeGnssMeasurementsListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeGnssMeasurementsListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean addGnssNavigationMessageListener(android.location.IGnssNavigationMessageListener listener, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addGnssNavigationMessageListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().addGnssNavigationMessageListener(listener, packageName);
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
      @Override public void removeGnssNavigationMessageListener(android.location.IGnssNavigationMessageListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeGnssNavigationMessageListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeGnssNavigationMessageListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getGnssYearOfHardware() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getGnssYearOfHardware, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getGnssYearOfHardware();
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.lang.String getGnssHardwareModelName() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getGnssHardwareModelName, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getGnssHardwareModelName();
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public int getGnssBatchSize(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getGnssBatchSize, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getGnssBatchSize(packageName);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public boolean addGnssBatchingCallback(android.location.IBatchedLocationCallback callback, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addGnssBatchingCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().addGnssBatchingCallback(callback, packageName);
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
      @Override public void removeGnssBatchingCallback() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeGnssBatchingCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeGnssBatchingCallback();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean startGnssBatch(long periodNanos, boolean wakeOnFifoFull, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(periodNanos);
          _data.writeInt(((wakeOnFifoFull)?(1):(0)));
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startGnssBatch, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startGnssBatch(periodNanos, wakeOnFifoFull, packageName);
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
      @Override public void flushGnssBatch(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_flushGnssBatch, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().flushGnssBatch(packageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean stopGnssBatch() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopGnssBatch, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().stopGnssBatch();
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
      @Override public boolean injectLocation(android.location.Location location) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((location!=null)) {
            _data.writeInt(1);
            location.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_injectLocation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().injectLocation(location);
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
      @Override public java.util.List<java.lang.String> getAllProviders() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<java.lang.String> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAllProviders, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAllProviders();
          }
          _reply.readException();
          _result = _reply.createStringArrayList();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.util.List<java.lang.String> getProviders(android.location.Criteria criteria, boolean enabledOnly) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<java.lang.String> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((criteria!=null)) {
            _data.writeInt(1);
            criteria.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((enabledOnly)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getProviders, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getProviders(criteria, enabledOnly);
          }
          _reply.readException();
          _result = _reply.createStringArrayList();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.lang.String getBestProvider(android.location.Criteria criteria, boolean enabledOnly) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((criteria!=null)) {
            _data.writeInt(1);
            criteria.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((enabledOnly)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getBestProvider, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getBestProvider(criteria, enabledOnly);
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public com.android.internal.location.ProviderProperties getProviderProperties(java.lang.String provider) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.internal.location.ProviderProperties _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(provider);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getProviderProperties, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getProviderProperties(provider);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = com.android.internal.location.ProviderProperties.CREATOR.createFromParcel(_reply);
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
      @Override public boolean isProviderPackage(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isProviderPackage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isProviderPackage(packageName);
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
      @Override public void setExtraLocationControllerPackage(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setExtraLocationControllerPackage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setExtraLocationControllerPackage(packageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.lang.String getExtraLocationControllerPackage() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getExtraLocationControllerPackage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getExtraLocationControllerPackage();
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setExtraLocationControllerPackageEnabled(boolean enabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setExtraLocationControllerPackageEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setExtraLocationControllerPackageEnabled(enabled);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isExtraLocationControllerPackageEnabled() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isExtraLocationControllerPackageEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isExtraLocationControllerPackageEnabled();
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
      @Override public boolean isProviderEnabledForUser(java.lang.String provider, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(provider);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isProviderEnabledForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isProviderEnabledForUser(provider, userId);
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
      @Override public boolean isLocationEnabledForUser(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isLocationEnabledForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isLocationEnabledForUser(userId);
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
      @Override public void addTestProvider(java.lang.String name, com.android.internal.location.ProviderProperties properties, java.lang.String opPackageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(name);
          if ((properties!=null)) {
            _data.writeInt(1);
            properties.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(opPackageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addTestProvider, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addTestProvider(name, properties, opPackageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeTestProvider(java.lang.String provider, java.lang.String opPackageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(provider);
          _data.writeString(opPackageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeTestProvider, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeTestProvider(provider, opPackageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setTestProviderLocation(java.lang.String provider, android.location.Location loc, java.lang.String opPackageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(provider);
          if ((loc!=null)) {
            _data.writeInt(1);
            loc.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(opPackageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setTestProviderLocation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setTestProviderLocation(provider, loc, opPackageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setTestProviderEnabled(java.lang.String provider, boolean enabled, java.lang.String opPackageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(provider);
          _data.writeInt(((enabled)?(1):(0)));
          _data.writeString(opPackageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setTestProviderEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setTestProviderEnabled(provider, enabled, opPackageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.util.List<android.location.LocationRequest> getTestProviderCurrentRequests(java.lang.String provider, java.lang.String opPackageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.location.LocationRequest> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(provider);
          _data.writeString(opPackageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTestProviderCurrentRequests, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getTestProviderCurrentRequests(provider, opPackageName);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.location.LocationRequest.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.location.LocationTime getGnssTimeMillis() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.location.LocationTime _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getGnssTimeMillis, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getGnssTimeMillis();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.location.LocationTime.CREATOR.createFromParcel(_reply);
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
      // --- deprecated ---

      @Override public void setTestProviderStatus(java.lang.String provider, int status, android.os.Bundle extras, long updateTime, java.lang.String opPackageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(provider);
          _data.writeInt(status);
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeLong(updateTime);
          _data.writeString(opPackageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setTestProviderStatus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setTestProviderStatus(provider, status, extras, updateTime, opPackageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean sendExtraCommand(java.lang.String provider, java.lang.String command, android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(provider);
          _data.writeString(command);
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendExtraCommand, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().sendExtraCommand(provider, command, extras);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
          if ((0!=_reply.readInt())) {
            extras.readFromParcel(_reply);
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      // --- internal ---
      // for reporting callback completion

      @Override public void locationCallbackFinished(android.location.ILocationListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_locationCallbackFinished, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().locationCallbackFinished(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // used by gts tests to verify whitelists

      @Override public java.lang.String[] getBackgroundThrottlingWhitelist() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getBackgroundThrottlingWhitelist, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getBackgroundThrottlingWhitelist();
          }
          _reply.readException();
          _result = _reply.createStringArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.lang.String[] getIgnoreSettingsWhitelist() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getIgnoreSettingsWhitelist, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getIgnoreSettingsWhitelist();
          }
          _reply.readException();
          _result = _reply.createStringArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static android.location.ILocationManager sDefaultImpl;
    }
    static final int TRANSACTION_requestLocationUpdates = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_removeUpdates = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_requestGeofence = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_removeGeofence = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getLastLocation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_registerGnssStatusCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_unregisterGnssStatusCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_geocoderIsPresent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_getFromLocation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_getFromLocationName = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_sendNiResponse = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_addGnssMeasurementsListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_injectGnssMeasurementCorrections = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_getGnssCapabilities = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_removeGnssMeasurementsListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_addGnssNavigationMessageListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_removeGnssNavigationMessageListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_getGnssYearOfHardware = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_getGnssHardwareModelName = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_getGnssBatchSize = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_addGnssBatchingCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_removeGnssBatchingCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_startGnssBatch = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_flushGnssBatch = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_stopGnssBatch = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_injectLocation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_getAllProviders = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    static final int TRANSACTION_getProviders = (android.os.IBinder.FIRST_CALL_TRANSACTION + 27);
    static final int TRANSACTION_getBestProvider = (android.os.IBinder.FIRST_CALL_TRANSACTION + 28);
    static final int TRANSACTION_getProviderProperties = (android.os.IBinder.FIRST_CALL_TRANSACTION + 29);
    static final int TRANSACTION_isProviderPackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 30);
    static final int TRANSACTION_setExtraLocationControllerPackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 31);
    static final int TRANSACTION_getExtraLocationControllerPackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 32);
    static final int TRANSACTION_setExtraLocationControllerPackageEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 33);
    static final int TRANSACTION_isExtraLocationControllerPackageEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 34);
    static final int TRANSACTION_isProviderEnabledForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 35);
    static final int TRANSACTION_isLocationEnabledForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 36);
    static final int TRANSACTION_addTestProvider = (android.os.IBinder.FIRST_CALL_TRANSACTION + 37);
    static final int TRANSACTION_removeTestProvider = (android.os.IBinder.FIRST_CALL_TRANSACTION + 38);
    static final int TRANSACTION_setTestProviderLocation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 39);
    static final int TRANSACTION_setTestProviderEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 40);
    static final int TRANSACTION_getTestProviderCurrentRequests = (android.os.IBinder.FIRST_CALL_TRANSACTION + 41);
    static final int TRANSACTION_getGnssTimeMillis = (android.os.IBinder.FIRST_CALL_TRANSACTION + 42);
    static final int TRANSACTION_setTestProviderStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 43);
    static final int TRANSACTION_sendExtraCommand = (android.os.IBinder.FIRST_CALL_TRANSACTION + 44);
    static final int TRANSACTION_locationCallbackFinished = (android.os.IBinder.FIRST_CALL_TRANSACTION + 45);
    static final int TRANSACTION_getBackgroundThrottlingWhitelist = (android.os.IBinder.FIRST_CALL_TRANSACTION + 46);
    static final int TRANSACTION_getIgnoreSettingsWhitelist = (android.os.IBinder.FIRST_CALL_TRANSACTION + 47);
    public static boolean setDefaultImpl(android.location.ILocationManager impl) {
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
    public static android.location.ILocationManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void requestLocationUpdates(android.location.LocationRequest request, android.location.ILocationListener listener, android.app.PendingIntent intent, java.lang.String packageName) throws android.os.RemoteException;
  public void removeUpdates(android.location.ILocationListener listener, android.app.PendingIntent intent, java.lang.String packageName) throws android.os.RemoteException;
  public void requestGeofence(android.location.LocationRequest request, android.location.Geofence geofence, android.app.PendingIntent intent, java.lang.String packageName) throws android.os.RemoteException;
  public void removeGeofence(android.location.Geofence fence, android.app.PendingIntent intent, java.lang.String packageName) throws android.os.RemoteException;
  public android.location.Location getLastLocation(android.location.LocationRequest request, java.lang.String packageName) throws android.os.RemoteException;
  public boolean registerGnssStatusCallback(android.location.IGnssStatusListener callback, java.lang.String packageName) throws android.os.RemoteException;
  public void unregisterGnssStatusCallback(android.location.IGnssStatusListener callback) throws android.os.RemoteException;
  public boolean geocoderIsPresent() throws android.os.RemoteException;
  public java.lang.String getFromLocation(double latitude, double longitude, int maxResults, android.location.GeocoderParams params, java.util.List<android.location.Address> addrs) throws android.os.RemoteException;
  public java.lang.String getFromLocationName(java.lang.String locationName, double lowerLeftLatitude, double lowerLeftLongitude, double upperRightLatitude, double upperRightLongitude, int maxResults, android.location.GeocoderParams params, java.util.List<android.location.Address> addrs) throws android.os.RemoteException;
  public boolean sendNiResponse(int notifId, int userResponse) throws android.os.RemoteException;
  public boolean addGnssMeasurementsListener(android.location.IGnssMeasurementsListener listener, java.lang.String packageName) throws android.os.RemoteException;
  public void injectGnssMeasurementCorrections(android.location.GnssMeasurementCorrections corrections, java.lang.String packageName) throws android.os.RemoteException;
  public long getGnssCapabilities(java.lang.String packageName) throws android.os.RemoteException;
  public void removeGnssMeasurementsListener(android.location.IGnssMeasurementsListener listener) throws android.os.RemoteException;
  public boolean addGnssNavigationMessageListener(android.location.IGnssNavigationMessageListener listener, java.lang.String packageName) throws android.os.RemoteException;
  public void removeGnssNavigationMessageListener(android.location.IGnssNavigationMessageListener listener) throws android.os.RemoteException;
  public int getGnssYearOfHardware() throws android.os.RemoteException;
  public java.lang.String getGnssHardwareModelName() throws android.os.RemoteException;
  public int getGnssBatchSize(java.lang.String packageName) throws android.os.RemoteException;
  public boolean addGnssBatchingCallback(android.location.IBatchedLocationCallback callback, java.lang.String packageName) throws android.os.RemoteException;
  public void removeGnssBatchingCallback() throws android.os.RemoteException;
  public boolean startGnssBatch(long periodNanos, boolean wakeOnFifoFull, java.lang.String packageName) throws android.os.RemoteException;
  public void flushGnssBatch(java.lang.String packageName) throws android.os.RemoteException;
  public boolean stopGnssBatch() throws android.os.RemoteException;
  public boolean injectLocation(android.location.Location location) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/location/java/android/location/ILocationManager.aidl:89:1:89:25")
  public java.util.List<java.lang.String> getAllProviders() throws android.os.RemoteException;
  public java.util.List<java.lang.String> getProviders(android.location.Criteria criteria, boolean enabledOnly) throws android.os.RemoteException;
  public java.lang.String getBestProvider(android.location.Criteria criteria, boolean enabledOnly) throws android.os.RemoteException;
  public com.android.internal.location.ProviderProperties getProviderProperties(java.lang.String provider) throws android.os.RemoteException;
  public boolean isProviderPackage(java.lang.String packageName) throws android.os.RemoteException;
  public void setExtraLocationControllerPackage(java.lang.String packageName) throws android.os.RemoteException;
  public java.lang.String getExtraLocationControllerPackage() throws android.os.RemoteException;
  public void setExtraLocationControllerPackageEnabled(boolean enabled) throws android.os.RemoteException;
  public boolean isExtraLocationControllerPackageEnabled() throws android.os.RemoteException;
  public boolean isProviderEnabledForUser(java.lang.String provider, int userId) throws android.os.RemoteException;
  public boolean isLocationEnabledForUser(int userId) throws android.os.RemoteException;
  public void addTestProvider(java.lang.String name, com.android.internal.location.ProviderProperties properties, java.lang.String opPackageName) throws android.os.RemoteException;
  public void removeTestProvider(java.lang.String provider, java.lang.String opPackageName) throws android.os.RemoteException;
  public void setTestProviderLocation(java.lang.String provider, android.location.Location loc, java.lang.String opPackageName) throws android.os.RemoteException;
  public void setTestProviderEnabled(java.lang.String provider, boolean enabled, java.lang.String opPackageName) throws android.os.RemoteException;
  public java.util.List<android.location.LocationRequest> getTestProviderCurrentRequests(java.lang.String provider, java.lang.String opPackageName) throws android.os.RemoteException;
  public android.location.LocationTime getGnssTimeMillis() throws android.os.RemoteException;
  // --- deprecated ---

  public void setTestProviderStatus(java.lang.String provider, int status, android.os.Bundle extras, long updateTime, java.lang.String opPackageName) throws android.os.RemoteException;
  public boolean sendExtraCommand(java.lang.String provider, java.lang.String command, android.os.Bundle extras) throws android.os.RemoteException;
  // --- internal ---
  // for reporting callback completion

  public void locationCallbackFinished(android.location.ILocationListener listener) throws android.os.RemoteException;
  // used by gts tests to verify whitelists

  public java.lang.String[] getBackgroundThrottlingWhitelist() throws android.os.RemoteException;
  public java.lang.String[] getIgnoreSettingsWhitelist() throws android.os.RemoteException;
}
