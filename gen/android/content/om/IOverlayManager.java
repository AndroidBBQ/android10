/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.content.om;
/**
 * Api for getting information about overlay packages.
 *
 * {@hide}
 */
public interface IOverlayManager extends android.os.IInterface
{
  /** Default implementation for IOverlayManager. */
  public static class Default implements android.content.om.IOverlayManager
  {
    /**
         * Returns information about all installed overlay packages for the
         * specified user. If there are no installed overlay packages for this user,
         * an empty map is returned (i.e. null is never returned). The returned map is a
         * mapping of target package names to lists of overlays. Each list for a
         * given target package is sorted in priority order, with the overlay with
         * the highest priority at the end of the list.
         *
         * @param userId The user to get the OverlayInfos for.
         * @return A Map<String, List<OverlayInfo>> with target package names
         *         mapped to lists of overlays; if no overlays exist for the
         *         requested user, an empty map is returned.
         */
    @Override public java.util.Map getAllOverlays(int userId) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Returns information about all overlays for the given target package for
         * the specified user. The returned list is ordered according to the
         * overlay priority with the highest priority at the end of the list.
         *
         * @param targetPackageName The name of the target package.
         * @param userId The user to get the OverlayInfos for.
         * @return A list of OverlayInfo objects; if no overlays exist for the
         *         requested package, an empty list is returned.
         */
    @Override public java.util.List getOverlayInfosForTarget(java.lang.String targetPackageName, int userId) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Returns information about the overlay with the given package name for the
         * specified user.
         *
         * @param packageName The name of the overlay package.
         * @param userId The user to get the OverlayInfo for.
         * @return The OverlayInfo for the overlay package; or null if no such
         *         overlay package exists.
         */
    @Override public android.content.om.OverlayInfo getOverlayInfo(java.lang.String packageName, int userId) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Request that an overlay package be enabled or disabled when possible to
         * do so.
         *
         * It is always possible to disable an overlay, but due to technical and
         * security reasons it may not always be possible to enable an overlay. An
         * example of the latter is when the related target package is not
         * installed. If the technical obstacle is later overcome, the overlay is
         * automatically enabled at that point in time.
         *
         * An enabled overlay is a part of target package's resources, i.e. it will
         * be part of any lookups performed via {@link android.content.res.Resources}
         * and {@link android.content.res.AssetManager}. A disabled overlay will no
         * longer affect the resources of the target package. If the target is
         * currently running, its outdated resources will be replaced by new ones.
         * This happens the same way as when an application enters or exits split
         * window mode.
         *
         * @param packageName The name of the overlay package.
         * @param enable true to enable the overlay, false to disable it.
         * @param userId The user for which to change the overlay.
         * @return true if the system successfully registered the request, false otherwise.
         */
    @Override public boolean setEnabled(java.lang.String packageName, boolean enable, int userId) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Request that an overlay package is enabled and any other overlay packages with the same
         * target package are disabled.
         *
         * See {@link #setEnabled} for the details on overlay packages.
         *
         * @param packageName the name of the overlay package to enable.
         * @param enabled must be true, otherwise the operation fails.
         * @param userId The user for which to change the overlay.
         * @return true if the system successfully registered the request, false otherwise.
         */
    @Override public boolean setEnabledExclusive(java.lang.String packageName, boolean enable, int userId) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Request that an overlay package is enabled and any other overlay packages with the same
         * target package and category are disabled.
         *
         * See {@link #setEnabled} for the details on overlay packages.
         *
         * @param packageName the name of the overlay package to enable.
         * @param userId The user for which to change the overlay.
         * @return true if the system successfully registered the request, false otherwise.
         */
    @Override public boolean setEnabledExclusiveInCategory(java.lang.String packageName, int userId) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Change the priority of the given overlay to be just higher than the
         * overlay with package name newParentPackageName. Both overlay packages
         * must have the same target and user.
         *
         * @see getOverlayInfosForTarget
         *
         * @param packageName The name of the overlay package whose priority should
         *        be adjusted.
         * @param newParentPackageName The name of the overlay package the newly
         *        adjusted overlay package should just outrank.
         * @param userId The user for which to change the overlay.
         */
    @Override public boolean setPriority(java.lang.String packageName, java.lang.String newParentPackageName, int userId) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Change the priority of the given overlay to the highest priority relative to
         * the other overlays with the same target and user.
         *
         * @see getOverlayInfosForTarget
         *
         * @param packageName The name of the overlay package whose priority should
         *        be adjusted.
         * @param userId The user for which to change the overlay.
         */
    @Override public boolean setHighestPriority(java.lang.String packageName, int userId) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Change the priority of the overlay to the lowest priority relative to
         * the other overlays for the same target and user.
         *
         * @see getOverlayInfosForTarget
         *
         * @param packageName The name of the overlay package whose priority should
         *        be adjusted.
         * @param userId The user for which to change the overlay.
         */
    @Override public boolean setLowestPriority(java.lang.String packageName, int userId) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Returns the list of default overlay packages, or an empty array if there are none.
         */
    @Override public java.lang.String[] getDefaultOverlayPackages() throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.content.om.IOverlayManager
  {
    private static final java.lang.String DESCRIPTOR = "android.content.om.IOverlayManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.content.om.IOverlayManager interface,
     * generating a proxy if needed.
     */
    public static android.content.om.IOverlayManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.content.om.IOverlayManager))) {
        return ((android.content.om.IOverlayManager)iin);
      }
      return new android.content.om.IOverlayManager.Stub.Proxy(obj);
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
        case TRANSACTION_getAllOverlays:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.util.Map _result = this.getAllOverlays(_arg0);
          reply.writeNoException();
          reply.writeMap(_result);
          return true;
        }
        case TRANSACTION_getOverlayInfosForTarget:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          java.util.List _result = this.getOverlayInfosForTarget(_arg0, _arg1);
          reply.writeNoException();
          reply.writeList(_result);
          return true;
        }
        case TRANSACTION_getOverlayInfo:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          android.content.om.OverlayInfo _result = this.getOverlayInfo(_arg0, _arg1);
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
        case TRANSACTION_setEnabled:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          int _arg2;
          _arg2 = data.readInt();
          boolean _result = this.setEnabled(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setEnabledExclusive:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          int _arg2;
          _arg2 = data.readInt();
          boolean _result = this.setEnabledExclusive(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setEnabledExclusiveInCategory:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.setEnabledExclusiveInCategory(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setPriority:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          boolean _result = this.setPriority(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setHighestPriority:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.setHighestPriority(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setLowestPriority:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.setLowestPriority(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getDefaultOverlayPackages:
        {
          data.enforceInterface(descriptor);
          java.lang.String[] _result = this.getDefaultOverlayPackages();
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
    private static class Proxy implements android.content.om.IOverlayManager
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
      /**
           * Returns information about all installed overlay packages for the
           * specified user. If there are no installed overlay packages for this user,
           * an empty map is returned (i.e. null is never returned). The returned map is a
           * mapping of target package names to lists of overlays. Each list for a
           * given target package is sorted in priority order, with the overlay with
           * the highest priority at the end of the list.
           *
           * @param userId The user to get the OverlayInfos for.
           * @return A Map<String, List<OverlayInfo>> with target package names
           *         mapped to lists of overlays; if no overlays exist for the
           *         requested user, an empty map is returned.
           */
      @Override public java.util.Map getAllOverlays(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.Map _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAllOverlays, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAllOverlays(userId);
          }
          _reply.readException();
          java.lang.ClassLoader cl = (java.lang.ClassLoader)this.getClass().getClassLoader();
          _result = _reply.readHashMap(cl);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Returns information about all overlays for the given target package for
           * the specified user. The returned list is ordered according to the
           * overlay priority with the highest priority at the end of the list.
           *
           * @param targetPackageName The name of the target package.
           * @param userId The user to get the OverlayInfos for.
           * @return A list of OverlayInfo objects; if no overlays exist for the
           *         requested package, an empty list is returned.
           */
      @Override public java.util.List getOverlayInfosForTarget(java.lang.String targetPackageName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(targetPackageName);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getOverlayInfosForTarget, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getOverlayInfosForTarget(targetPackageName, userId);
          }
          _reply.readException();
          java.lang.ClassLoader cl = (java.lang.ClassLoader)this.getClass().getClassLoader();
          _result = _reply.readArrayList(cl);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Returns information about the overlay with the given package name for the
           * specified user.
           *
           * @param packageName The name of the overlay package.
           * @param userId The user to get the OverlayInfo for.
           * @return The OverlayInfo for the overlay package; or null if no such
           *         overlay package exists.
           */
      @Override public android.content.om.OverlayInfo getOverlayInfo(java.lang.String packageName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.om.OverlayInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getOverlayInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getOverlayInfo(packageName, userId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.content.om.OverlayInfo.CREATOR.createFromParcel(_reply);
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
      /**
           * Request that an overlay package be enabled or disabled when possible to
           * do so.
           *
           * It is always possible to disable an overlay, but due to technical and
           * security reasons it may not always be possible to enable an overlay. An
           * example of the latter is when the related target package is not
           * installed. If the technical obstacle is later overcome, the overlay is
           * automatically enabled at that point in time.
           *
           * An enabled overlay is a part of target package's resources, i.e. it will
           * be part of any lookups performed via {@link android.content.res.Resources}
           * and {@link android.content.res.AssetManager}. A disabled overlay will no
           * longer affect the resources of the target package. If the target is
           * currently running, its outdated resources will be replaced by new ones.
           * This happens the same way as when an application enters or exits split
           * window mode.
           *
           * @param packageName The name of the overlay package.
           * @param enable true to enable the overlay, false to disable it.
           * @param userId The user for which to change the overlay.
           * @return true if the system successfully registered the request, false otherwise.
           */
      @Override public boolean setEnabled(java.lang.String packageName, boolean enable, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(((enable)?(1):(0)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setEnabled(packageName, enable, userId);
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
           * Request that an overlay package is enabled and any other overlay packages with the same
           * target package are disabled.
           *
           * See {@link #setEnabled} for the details on overlay packages.
           *
           * @param packageName the name of the overlay package to enable.
           * @param enabled must be true, otherwise the operation fails.
           * @param userId The user for which to change the overlay.
           * @return true if the system successfully registered the request, false otherwise.
           */
      @Override public boolean setEnabledExclusive(java.lang.String packageName, boolean enable, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(((enable)?(1):(0)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setEnabledExclusive, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setEnabledExclusive(packageName, enable, userId);
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
           * Request that an overlay package is enabled and any other overlay packages with the same
           * target package and category are disabled.
           *
           * See {@link #setEnabled} for the details on overlay packages.
           *
           * @param packageName the name of the overlay package to enable.
           * @param userId The user for which to change the overlay.
           * @return true if the system successfully registered the request, false otherwise.
           */
      @Override public boolean setEnabledExclusiveInCategory(java.lang.String packageName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setEnabledExclusiveInCategory, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setEnabledExclusiveInCategory(packageName, userId);
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
           * Change the priority of the given overlay to be just higher than the
           * overlay with package name newParentPackageName. Both overlay packages
           * must have the same target and user.
           *
           * @see getOverlayInfosForTarget
           *
           * @param packageName The name of the overlay package whose priority should
           *        be adjusted.
           * @param newParentPackageName The name of the overlay package the newly
           *        adjusted overlay package should just outrank.
           * @param userId The user for which to change the overlay.
           */
      @Override public boolean setPriority(java.lang.String packageName, java.lang.String newParentPackageName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeString(newParentPackageName);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setPriority, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setPriority(packageName, newParentPackageName, userId);
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
           * Change the priority of the given overlay to the highest priority relative to
           * the other overlays with the same target and user.
           *
           * @see getOverlayInfosForTarget
           *
           * @param packageName The name of the overlay package whose priority should
           *        be adjusted.
           * @param userId The user for which to change the overlay.
           */
      @Override public boolean setHighestPriority(java.lang.String packageName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setHighestPriority, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setHighestPriority(packageName, userId);
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
           * Change the priority of the overlay to the lowest priority relative to
           * the other overlays for the same target and user.
           *
           * @see getOverlayInfosForTarget
           *
           * @param packageName The name of the overlay package whose priority should
           *        be adjusted.
           * @param userId The user for which to change the overlay.
           */
      @Override public boolean setLowestPriority(java.lang.String packageName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setLowestPriority, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setLowestPriority(packageName, userId);
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
           * Returns the list of default overlay packages, or an empty array if there are none.
           */
      @Override public java.lang.String[] getDefaultOverlayPackages() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDefaultOverlayPackages, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDefaultOverlayPackages();
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
      public static android.content.om.IOverlayManager sDefaultImpl;
    }
    static final int TRANSACTION_getAllOverlays = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getOverlayInfosForTarget = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getOverlayInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_setEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_setEnabledExclusive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_setEnabledExclusiveInCategory = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_setPriority = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_setHighestPriority = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_setLowestPriority = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_getDefaultOverlayPackages = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    public static boolean setDefaultImpl(android.content.om.IOverlayManager impl) {
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
    public static android.content.om.IOverlayManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Returns information about all installed overlay packages for the
       * specified user. If there are no installed overlay packages for this user,
       * an empty map is returned (i.e. null is never returned). The returned map is a
       * mapping of target package names to lists of overlays. Each list for a
       * given target package is sorted in priority order, with the overlay with
       * the highest priority at the end of the list.
       *
       * @param userId The user to get the OverlayInfos for.
       * @return A Map<String, List<OverlayInfo>> with target package names
       *         mapped to lists of overlays; if no overlays exist for the
       *         requested user, an empty map is returned.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/content/om/IOverlayManager.aidl:40:1:40:25")
  public java.util.Map getAllOverlays(int userId) throws android.os.RemoteException;
  /**
       * Returns information about all overlays for the given target package for
       * the specified user. The returned list is ordered according to the
       * overlay priority with the highest priority at the end of the list.
       *
       * @param targetPackageName The name of the target package.
       * @param userId The user to get the OverlayInfos for.
       * @return A list of OverlayInfo objects; if no overlays exist for the
       *         requested package, an empty list is returned.
       */
  public java.util.List getOverlayInfosForTarget(java.lang.String targetPackageName, int userId) throws android.os.RemoteException;
  /**
       * Returns information about the overlay with the given package name for the
       * specified user.
       *
       * @param packageName The name of the overlay package.
       * @param userId The user to get the OverlayInfo for.
       * @return The OverlayInfo for the overlay package; or null if no such
       *         overlay package exists.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/content/om/IOverlayManager.aidl:64:1:64:25")
  public android.content.om.OverlayInfo getOverlayInfo(java.lang.String packageName, int userId) throws android.os.RemoteException;
  /**
       * Request that an overlay package be enabled or disabled when possible to
       * do so.
       *
       * It is always possible to disable an overlay, but due to technical and
       * security reasons it may not always be possible to enable an overlay. An
       * example of the latter is when the related target package is not
       * installed. If the technical obstacle is later overcome, the overlay is
       * automatically enabled at that point in time.
       *
       * An enabled overlay is a part of target package's resources, i.e. it will
       * be part of any lookups performed via {@link android.content.res.Resources}
       * and {@link android.content.res.AssetManager}. A disabled overlay will no
       * longer affect the resources of the target package. If the target is
       * currently running, its outdated resources will be replaced by new ones.
       * This happens the same way as when an application enters or exits split
       * window mode.
       *
       * @param packageName The name of the overlay package.
       * @param enable true to enable the overlay, false to disable it.
       * @param userId The user for which to change the overlay.
       * @return true if the system successfully registered the request, false otherwise.
       */
  public boolean setEnabled(java.lang.String packageName, boolean enable, int userId) throws android.os.RemoteException;
  /**
       * Request that an overlay package is enabled and any other overlay packages with the same
       * target package are disabled.
       *
       * See {@link #setEnabled} for the details on overlay packages.
       *
       * @param packageName the name of the overlay package to enable.
       * @param enabled must be true, otherwise the operation fails.
       * @param userId The user for which to change the overlay.
       * @return true if the system successfully registered the request, false otherwise.
       */
  public boolean setEnabledExclusive(java.lang.String packageName, boolean enable, int userId) throws android.os.RemoteException;
  /**
       * Request that an overlay package is enabled and any other overlay packages with the same
       * target package and category are disabled.
       *
       * See {@link #setEnabled} for the details on overlay packages.
       *
       * @param packageName the name of the overlay package to enable.
       * @param userId The user for which to change the overlay.
       * @return true if the system successfully registered the request, false otherwise.
       */
  public boolean setEnabledExclusiveInCategory(java.lang.String packageName, int userId) throws android.os.RemoteException;
  /**
       * Change the priority of the given overlay to be just higher than the
       * overlay with package name newParentPackageName. Both overlay packages
       * must have the same target and user.
       *
       * @see getOverlayInfosForTarget
       *
       * @param packageName The name of the overlay package whose priority should
       *        be adjusted.
       * @param newParentPackageName The name of the overlay package the newly
       *        adjusted overlay package should just outrank.
       * @param userId The user for which to change the overlay.
       */
  public boolean setPriority(java.lang.String packageName, java.lang.String newParentPackageName, int userId) throws android.os.RemoteException;
  /**
       * Change the priority of the given overlay to the highest priority relative to
       * the other overlays with the same target and user.
       *
       * @see getOverlayInfosForTarget
       *
       * @param packageName The name of the overlay package whose priority should
       *        be adjusted.
       * @param userId The user for which to change the overlay.
       */
  public boolean setHighestPriority(java.lang.String packageName, int userId) throws android.os.RemoteException;
  /**
       * Change the priority of the overlay to the lowest priority relative to
       * the other overlays for the same target and user.
       *
       * @see getOverlayInfosForTarget
       *
       * @param packageName The name of the overlay package whose priority should
       *        be adjusted.
       * @param userId The user for which to change the overlay.
       */
  public boolean setLowestPriority(java.lang.String packageName, int userId) throws android.os.RemoteException;
  /**
       * Returns the list of default overlay packages, or an empty array if there are none.
       */
  public java.lang.String[] getDefaultOverlayPackages() throws android.os.RemoteException;
}
