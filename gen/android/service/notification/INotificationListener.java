/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.notification;
/** @hide */
public interface INotificationListener extends android.os.IInterface
{
  /** Default implementation for INotificationListener. */
  public static class Default implements android.service.notification.INotificationListener
  {
    // listeners and assistant

    @Override public void onListenerConnected(android.service.notification.NotificationRankingUpdate update) throws android.os.RemoteException
    {
    }
    @Override public void onNotificationPosted(android.service.notification.IStatusBarNotificationHolder notificationHolder, android.service.notification.NotificationRankingUpdate update) throws android.os.RemoteException
    {
    }
    @Override public void onStatusBarIconsBehaviorChanged(boolean hideSilentStatusIcons) throws android.os.RemoteException
    {
    }
    // stats only for assistant

    @Override public void onNotificationRemoved(android.service.notification.IStatusBarNotificationHolder notificationHolder, android.service.notification.NotificationRankingUpdate update, android.service.notification.NotificationStats stats, int reason) throws android.os.RemoteException
    {
    }
    @Override public void onNotificationRankingUpdate(android.service.notification.NotificationRankingUpdate update) throws android.os.RemoteException
    {
    }
    @Override public void onListenerHintsChanged(int hints) throws android.os.RemoteException
    {
    }
    @Override public void onInterruptionFilterChanged(int interruptionFilter) throws android.os.RemoteException
    {
    }
    // companion device managers only

    @Override public void onNotificationChannelModification(java.lang.String pkgName, android.os.UserHandle user, android.app.NotificationChannel channel, int modificationType) throws android.os.RemoteException
    {
    }
    @Override public void onNotificationChannelGroupModification(java.lang.String pkgName, android.os.UserHandle user, android.app.NotificationChannelGroup group, int modificationType) throws android.os.RemoteException
    {
    }
    // assistants only

    @Override public void onNotificationEnqueuedWithChannel(android.service.notification.IStatusBarNotificationHolder notificationHolder, android.app.NotificationChannel channel) throws android.os.RemoteException
    {
    }
    @Override public void onNotificationSnoozedUntilContext(android.service.notification.IStatusBarNotificationHolder notificationHolder, java.lang.String snoozeCriterionId) throws android.os.RemoteException
    {
    }
    @Override public void onNotificationsSeen(java.util.List<java.lang.String> keys) throws android.os.RemoteException
    {
    }
    @Override public void onNotificationExpansionChanged(java.lang.String key, boolean userAction, boolean expanded) throws android.os.RemoteException
    {
    }
    @Override public void onNotificationDirectReply(java.lang.String key) throws android.os.RemoteException
    {
    }
    @Override public void onSuggestedReplySent(java.lang.String key, java.lang.CharSequence reply, int source) throws android.os.RemoteException
    {
    }
    @Override public void onActionClicked(java.lang.String key, android.app.Notification.Action action, int source) throws android.os.RemoteException
    {
    }
    @Override public void onAllowedAdjustmentsChanged() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.notification.INotificationListener
  {
    private static final java.lang.String DESCRIPTOR = "android.service.notification.INotificationListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.notification.INotificationListener interface,
     * generating a proxy if needed.
     */
    public static android.service.notification.INotificationListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.notification.INotificationListener))) {
        return ((android.service.notification.INotificationListener)iin);
      }
      return new android.service.notification.INotificationListener.Stub.Proxy(obj);
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
        case TRANSACTION_onListenerConnected:
        {
          data.enforceInterface(descriptor);
          android.service.notification.NotificationRankingUpdate _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.service.notification.NotificationRankingUpdate.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onListenerConnected(_arg0);
          return true;
        }
        case TRANSACTION_onNotificationPosted:
        {
          data.enforceInterface(descriptor);
          android.service.notification.IStatusBarNotificationHolder _arg0;
          _arg0 = android.service.notification.IStatusBarNotificationHolder.Stub.asInterface(data.readStrongBinder());
          android.service.notification.NotificationRankingUpdate _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.service.notification.NotificationRankingUpdate.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.onNotificationPosted(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onStatusBarIconsBehaviorChanged:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.onStatusBarIconsBehaviorChanged(_arg0);
          return true;
        }
        case TRANSACTION_onNotificationRemoved:
        {
          data.enforceInterface(descriptor);
          android.service.notification.IStatusBarNotificationHolder _arg0;
          _arg0 = android.service.notification.IStatusBarNotificationHolder.Stub.asInterface(data.readStrongBinder());
          android.service.notification.NotificationRankingUpdate _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.service.notification.NotificationRankingUpdate.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.service.notification.NotificationStats _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.service.notification.NotificationStats.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          this.onNotificationRemoved(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_onNotificationRankingUpdate:
        {
          data.enforceInterface(descriptor);
          android.service.notification.NotificationRankingUpdate _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.service.notification.NotificationRankingUpdate.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onNotificationRankingUpdate(_arg0);
          return true;
        }
        case TRANSACTION_onListenerHintsChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onListenerHintsChanged(_arg0);
          return true;
        }
        case TRANSACTION_onInterruptionFilterChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onInterruptionFilterChanged(_arg0);
          return true;
        }
        case TRANSACTION_onNotificationChannelModification:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.os.UserHandle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.UserHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.app.NotificationChannel _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.app.NotificationChannel.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          this.onNotificationChannelModification(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_onNotificationChannelGroupModification:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.os.UserHandle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.UserHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.app.NotificationChannelGroup _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.app.NotificationChannelGroup.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          this.onNotificationChannelGroupModification(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_onNotificationEnqueuedWithChannel:
        {
          data.enforceInterface(descriptor);
          android.service.notification.IStatusBarNotificationHolder _arg0;
          _arg0 = android.service.notification.IStatusBarNotificationHolder.Stub.asInterface(data.readStrongBinder());
          android.app.NotificationChannel _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.app.NotificationChannel.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.onNotificationEnqueuedWithChannel(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onNotificationSnoozedUntilContext:
        {
          data.enforceInterface(descriptor);
          android.service.notification.IStatusBarNotificationHolder _arg0;
          _arg0 = android.service.notification.IStatusBarNotificationHolder.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.onNotificationSnoozedUntilContext(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onNotificationsSeen:
        {
          data.enforceInterface(descriptor);
          java.util.List<java.lang.String> _arg0;
          _arg0 = data.createStringArrayList();
          this.onNotificationsSeen(_arg0);
          return true;
        }
        case TRANSACTION_onNotificationExpansionChanged:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          this.onNotificationExpansionChanged(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onNotificationDirectReply:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.onNotificationDirectReply(_arg0);
          return true;
        }
        case TRANSACTION_onSuggestedReplySent:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.CharSequence _arg1;
          if (0!=data.readInt()) {
            _arg1 = android.text.TextUtils.CHAR_SEQUENCE_CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          this.onSuggestedReplySent(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onActionClicked:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.app.Notification.Action _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.app.Notification.Action.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          this.onActionClicked(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onAllowedAdjustmentsChanged:
        {
          data.enforceInterface(descriptor);
          this.onAllowedAdjustmentsChanged();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.notification.INotificationListener
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
      // listeners and assistant

      @Override public void onListenerConnected(android.service.notification.NotificationRankingUpdate update) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((update!=null)) {
            _data.writeInt(1);
            update.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onListenerConnected, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onListenerConnected(update);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onNotificationPosted(android.service.notification.IStatusBarNotificationHolder notificationHolder, android.service.notification.NotificationRankingUpdate update) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((notificationHolder!=null))?(notificationHolder.asBinder()):(null)));
          if ((update!=null)) {
            _data.writeInt(1);
            update.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNotificationPosted, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNotificationPosted(notificationHolder, update);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onStatusBarIconsBehaviorChanged(boolean hideSilentStatusIcons) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((hideSilentStatusIcons)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onStatusBarIconsBehaviorChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onStatusBarIconsBehaviorChanged(hideSilentStatusIcons);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // stats only for assistant

      @Override public void onNotificationRemoved(android.service.notification.IStatusBarNotificationHolder notificationHolder, android.service.notification.NotificationRankingUpdate update, android.service.notification.NotificationStats stats, int reason) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((notificationHolder!=null))?(notificationHolder.asBinder()):(null)));
          if ((update!=null)) {
            _data.writeInt(1);
            update.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((stats!=null)) {
            _data.writeInt(1);
            stats.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(reason);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNotificationRemoved, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNotificationRemoved(notificationHolder, update, stats, reason);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onNotificationRankingUpdate(android.service.notification.NotificationRankingUpdate update) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((update!=null)) {
            _data.writeInt(1);
            update.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNotificationRankingUpdate, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNotificationRankingUpdate(update);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onListenerHintsChanged(int hints) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(hints);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onListenerHintsChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onListenerHintsChanged(hints);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onInterruptionFilterChanged(int interruptionFilter) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(interruptionFilter);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onInterruptionFilterChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onInterruptionFilterChanged(interruptionFilter);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // companion device managers only

      @Override public void onNotificationChannelModification(java.lang.String pkgName, android.os.UserHandle user, android.app.NotificationChannel channel, int modificationType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(pkgName);
          if ((user!=null)) {
            _data.writeInt(1);
            user.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((channel!=null)) {
            _data.writeInt(1);
            channel.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(modificationType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNotificationChannelModification, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNotificationChannelModification(pkgName, user, channel, modificationType);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onNotificationChannelGroupModification(java.lang.String pkgName, android.os.UserHandle user, android.app.NotificationChannelGroup group, int modificationType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(pkgName);
          if ((user!=null)) {
            _data.writeInt(1);
            user.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((group!=null)) {
            _data.writeInt(1);
            group.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(modificationType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNotificationChannelGroupModification, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNotificationChannelGroupModification(pkgName, user, group, modificationType);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // assistants only

      @Override public void onNotificationEnqueuedWithChannel(android.service.notification.IStatusBarNotificationHolder notificationHolder, android.app.NotificationChannel channel) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((notificationHolder!=null))?(notificationHolder.asBinder()):(null)));
          if ((channel!=null)) {
            _data.writeInt(1);
            channel.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNotificationEnqueuedWithChannel, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNotificationEnqueuedWithChannel(notificationHolder, channel);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onNotificationSnoozedUntilContext(android.service.notification.IStatusBarNotificationHolder notificationHolder, java.lang.String snoozeCriterionId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((notificationHolder!=null))?(notificationHolder.asBinder()):(null)));
          _data.writeString(snoozeCriterionId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNotificationSnoozedUntilContext, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNotificationSnoozedUntilContext(notificationHolder, snoozeCriterionId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onNotificationsSeen(java.util.List<java.lang.String> keys) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStringList(keys);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNotificationsSeen, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNotificationsSeen(keys);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onNotificationExpansionChanged(java.lang.String key, boolean userAction, boolean expanded) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(key);
          _data.writeInt(((userAction)?(1):(0)));
          _data.writeInt(((expanded)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNotificationExpansionChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNotificationExpansionChanged(key, userAction, expanded);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onNotificationDirectReply(java.lang.String key) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(key);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNotificationDirectReply, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNotificationDirectReply(key);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onSuggestedReplySent(java.lang.String key, java.lang.CharSequence reply, int source) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(key);
          if (reply!=null) {
            _data.writeInt(1);
            android.text.TextUtils.writeToParcel(reply, _data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(source);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSuggestedReplySent, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSuggestedReplySent(key, reply, source);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onActionClicked(java.lang.String key, android.app.Notification.Action action, int source) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(key);
          if ((action!=null)) {
            _data.writeInt(1);
            action.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(source);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onActionClicked, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onActionClicked(key, action, source);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onAllowedAdjustmentsChanged() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onAllowedAdjustmentsChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onAllowedAdjustmentsChanged();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.service.notification.INotificationListener sDefaultImpl;
    }
    static final int TRANSACTION_onListenerConnected = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onNotificationPosted = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onStatusBarIconsBehaviorChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onNotificationRemoved = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onNotificationRankingUpdate = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_onListenerHintsChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_onInterruptionFilterChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_onNotificationChannelModification = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_onNotificationChannelGroupModification = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_onNotificationEnqueuedWithChannel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_onNotificationSnoozedUntilContext = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_onNotificationsSeen = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_onNotificationExpansionChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_onNotificationDirectReply = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_onSuggestedReplySent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_onActionClicked = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_onAllowedAdjustmentsChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    public static boolean setDefaultImpl(android.service.notification.INotificationListener impl) {
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
    public static android.service.notification.INotificationListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  // listeners and assistant

  public void onListenerConnected(android.service.notification.NotificationRankingUpdate update) throws android.os.RemoteException;
  public void onNotificationPosted(android.service.notification.IStatusBarNotificationHolder notificationHolder, android.service.notification.NotificationRankingUpdate update) throws android.os.RemoteException;
  public void onStatusBarIconsBehaviorChanged(boolean hideSilentStatusIcons) throws android.os.RemoteException;
  // stats only for assistant

  public void onNotificationRemoved(android.service.notification.IStatusBarNotificationHolder notificationHolder, android.service.notification.NotificationRankingUpdate update, android.service.notification.NotificationStats stats, int reason) throws android.os.RemoteException;
  public void onNotificationRankingUpdate(android.service.notification.NotificationRankingUpdate update) throws android.os.RemoteException;
  public void onListenerHintsChanged(int hints) throws android.os.RemoteException;
  public void onInterruptionFilterChanged(int interruptionFilter) throws android.os.RemoteException;
  // companion device managers only

  public void onNotificationChannelModification(java.lang.String pkgName, android.os.UserHandle user, android.app.NotificationChannel channel, int modificationType) throws android.os.RemoteException;
  public void onNotificationChannelGroupModification(java.lang.String pkgName, android.os.UserHandle user, android.app.NotificationChannelGroup group, int modificationType) throws android.os.RemoteException;
  // assistants only

  public void onNotificationEnqueuedWithChannel(android.service.notification.IStatusBarNotificationHolder notificationHolder, android.app.NotificationChannel channel) throws android.os.RemoteException;
  public void onNotificationSnoozedUntilContext(android.service.notification.IStatusBarNotificationHolder notificationHolder, java.lang.String snoozeCriterionId) throws android.os.RemoteException;
  public void onNotificationsSeen(java.util.List<java.lang.String> keys) throws android.os.RemoteException;
  public void onNotificationExpansionChanged(java.lang.String key, boolean userAction, boolean expanded) throws android.os.RemoteException;
  public void onNotificationDirectReply(java.lang.String key) throws android.os.RemoteException;
  public void onSuggestedReplySent(java.lang.String key, java.lang.CharSequence reply, int source) throws android.os.RemoteException;
  public void onActionClicked(java.lang.String key, android.app.Notification.Action action, int source) throws android.os.RemoteException;
  public void onAllowedAdjustmentsChanged() throws android.os.RemoteException;
}
