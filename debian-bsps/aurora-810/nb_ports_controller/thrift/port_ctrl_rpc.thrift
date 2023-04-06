namespace py sn
namespace cpp sn

typedef bool nb_port_presence_t
typedef bool nb_port_lpmode_t
typedef i32 nb_ports_ctrl_ports_presence_t
typedef i32 nb_ports_ctrl_ports_lpmode_t

exception InvalidPortCtrlOperation {
  1 : i32 code
}

service PortCtrl {
    nb_ports_ctrl_ports_presence_t nb_all_presence_get() throws (1:InvalidPortCtrlOperation ouch);
    nb_port_presence_t nb_port_presence_get(1:i32 port_num) throws (1:InvalidPortCtrlOperation ouch);
    nb_ports_ctrl_ports_lpmode_t nb_all_lpmode_get() throws (1:InvalidPortCtrlOperation ouch);
    nb_port_lpmode_t nb_port_lpmode_get(1:i32 port_num) throws (1:InvalidPortCtrlOperation ouch);
    void nb_port_lpmode_set(1:i32 port_num 2:bool lpmode) throws(1:InvalidPortCtrlOperation ouch);
    void nb_port_reset(1:i32 port_num 2:bool reset) throws (1:InvalidPortCtrlOperation ouch);
    void nb_port_led_set(1:i32 port_num 2:i8 led_mode) throws (1:InvalidPortCtrlOperation ouch);
    binary nb_port_eeprom_get(1:i32 port_num 2:i16 offset 3:i16 length) throws (1:InvalidPortCtrlOperation ouch);
    void nb_port_eeprom_set(1:i32 port_num 2:i16 offset 3:binary data) throws (1:InvalidPortCtrlOperation ouch);
}
