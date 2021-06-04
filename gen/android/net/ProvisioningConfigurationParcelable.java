/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net;
public class ProvisioningConfigurationParcelable implements android.os.Parcelable
{

  public boolean enableIPv4;

  public boolean enableIPv6;

  public boolean usingMultinetworkPolicyTracker;

  public boolean usingIpReachabilityMonitor;

  public int requestedPreDhcpActionMs;

  public android.net.InitialConfigurationParcelable initialConfig;

  public android.net.StaticIpConfiguration staticIpConfig;

  public android.net.apf.ApfCapabilities apfCapabilities;

  public int provisioningTimeoutMs;

  public int ipv6AddrGenMode;

  public android.net.Network network;

  public java.lang.String displayName;
  public static final android.os.Parcelable.Creator<ProvisioningConfigurationParcelable> CREATOR = new android.os.Parcelable.Creator<ProvisioningConfigurationParcelable>() {
    @Override
    public ProvisioningConfigurationParcelable createFromParcel(android.os.Parcel _aidl_source) {
      ProvisioningConfigurationParcelable _aidl_out = new ProvisioningConfigurationParcelable();
      _aidl_out.readFromParcel(_aidl_source);
      return _aidl_out;
    }
    @Override
    public ProvisioningConfigurationParcelable[] newArray(int _aidl_size) {
      return new ProvisioningConfigurationParcelable[_aidl_size];
    }
  };
  @Override public final void writeToParcel(android.os.Parcel _aidl_parcel, int _aidl_flag)
  {
    int _aidl_start_pos = _aidl_parcel.dataPosition();
    _aidl_parcel.writeInt(0);
    _aidl_parcel.writeInt(((enableIPv4)?(1):(0)));
    _aidl_parcel.writeInt(((enableIPv6)?(1):(0)));
    _aidl_parcel.writeInt(((usingMultinetworkPolicyTracker)?(1):(0)));
    _aidl_parcel.writeInt(((usingIpReachabilityMonitor)?(1):(0)));
    _aidl_parcel.writeInt(requestedPreDhcpActionMs);
    if ((initialConfig!=null)) {
      _aidl_parcel.writeInt(1);
      initialConfig.writeToParcel(_aidl_parcel, 0);
    }
    else {
      _aidl_parcel.writeInt(0);
    }
    if ((staticIpConfig!=null)) {
      _aidl_parcel.writeInt(1);
      staticIpConfig.writeToParcel(_aidl_parcel, 0);
    }
    else {
      _aidl_parcel.writeInt(0);
    }
    if ((apfCapabilities!=null)) {
      _aidl_parcel.writeInt(1);
      apfCapabilities.writeToParcel(_aidl_parcel, 0);
    }
    else {
      _aidl_parcel.writeInt(0);
    }
    _aidl_parcel.writeInt(provisioningTimeoutMs);
    _aidl_parcel.writeInt(ipv6AddrGenMode);
    if ((network!=null)) {
      _aidl_parcel.writeInt(1);
      network.writeToParcel(_aidl_parcel, 0);
    }
    else {
      _aidl_parcel.writeInt(0);
    }
    _aidl_parcel.writeString(displayName);
    int _aidl_end_pos = _aidl_parcel.dataPosition();
    _aidl_parcel.setDataPosition(_aidl_start_pos);
    _aidl_parcel.writeInt(_aidl_end_pos - _aidl_start_pos);
    _aidl_parcel.setDataPosition(_aidl_end_pos);
  }
  public final void readFromParcel(android.os.Parcel _aidl_parcel)
  {
    int _aidl_start_pos = _aidl_parcel.dataPosition();
    int _aidl_parcelable_size = _aidl_parcel.readInt();
    if (_aidl_parcelable_size < 0) return;
    try {
      enableIPv4 = (0!=_aidl_parcel.readInt());
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      enableIPv6 = (0!=_aidl_parcel.readInt());
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      usingMultinetworkPolicyTracker = (0!=_aidl_parcel.readInt());
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      usingIpReachabilityMonitor = (0!=_aidl_parcel.readInt());
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      requestedPreDhcpActionMs = _aidl_parcel.readInt();
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      if ((0!=_aidl_parcel.readInt())) {
        initialConfig = android.net.InitialConfigurationParcelable.CREATOR.createFromParcel(_aidl_parcel);
      }
      else {
        initialConfig = null;
      }
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      if ((0!=_aidl_parcel.readInt())) {
        staticIpConfig = android.net.StaticIpConfiguration.CREATOR.createFromParcel(_aidl_parcel);
      }
      else {
        staticIpConfig = null;
      }
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      if ((0!=_aidl_parcel.readInt())) {
        apfCapabilities = android.net.apf.ApfCapabilities.CREATOR.createFromParcel(_aidl_parcel);
      }
      else {
        apfCapabilities = null;
      }
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      provisioningTimeoutMs = _aidl_parcel.readInt();
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      ipv6AddrGenMode = _aidl_parcel.readInt();
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      if ((0!=_aidl_parcel.readInt())) {
        network = android.net.Network.CREATOR.createFromParcel(_aidl_parcel);
      }
      else {
        network = null;
      }
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      displayName = _aidl_parcel.readString();
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
    } finally {
      _aidl_parcel.setDataPosition(_aidl_start_pos + _aidl_parcelable_size);
    }
  }
  @Override public int describeContents()
  {
    return 0;
  }
}
