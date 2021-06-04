/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net;
public class TcpKeepalivePacketDataParcelable implements android.os.Parcelable
{

  public byte[] srcAddress;

  public int srcPort;

  public byte[] dstAddress;

  public int dstPort;

  public int seq;

  public int ack;

  public int rcvWnd;

  public int rcvWndScale;

  public int tos;

  public int ttl;
  public static final android.os.Parcelable.Creator<TcpKeepalivePacketDataParcelable> CREATOR = new android.os.Parcelable.Creator<TcpKeepalivePacketDataParcelable>() {
    @Override
    public TcpKeepalivePacketDataParcelable createFromParcel(android.os.Parcel _aidl_source) {
      TcpKeepalivePacketDataParcelable _aidl_out = new TcpKeepalivePacketDataParcelable();
      _aidl_out.readFromParcel(_aidl_source);
      return _aidl_out;
    }
    @Override
    public TcpKeepalivePacketDataParcelable[] newArray(int _aidl_size) {
      return new TcpKeepalivePacketDataParcelable[_aidl_size];
    }
  };
  @Override public final void writeToParcel(android.os.Parcel _aidl_parcel, int _aidl_flag)
  {
    int _aidl_start_pos = _aidl_parcel.dataPosition();
    _aidl_parcel.writeInt(0);
    _aidl_parcel.writeByteArray(srcAddress);
    _aidl_parcel.writeInt(srcPort);
    _aidl_parcel.writeByteArray(dstAddress);
    _aidl_parcel.writeInt(dstPort);
    _aidl_parcel.writeInt(seq);
    _aidl_parcel.writeInt(ack);
    _aidl_parcel.writeInt(rcvWnd);
    _aidl_parcel.writeInt(rcvWndScale);
    _aidl_parcel.writeInt(tos);
    _aidl_parcel.writeInt(ttl);
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
      srcAddress = _aidl_parcel.createByteArray();
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      srcPort = _aidl_parcel.readInt();
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      dstAddress = _aidl_parcel.createByteArray();
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      dstPort = _aidl_parcel.readInt();
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      seq = _aidl_parcel.readInt();
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      ack = _aidl_parcel.readInt();
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      rcvWnd = _aidl_parcel.readInt();
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      rcvWndScale = _aidl_parcel.readInt();
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      tos = _aidl_parcel.readInt();
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      ttl = _aidl_parcel.readInt();
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
