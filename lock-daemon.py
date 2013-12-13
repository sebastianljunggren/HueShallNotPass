import dbus
import serial

bus = dbus.SessionBus()
ser = serial.Serial(port='/dev/ttyACM0')
screensaver = bus.get_object('org.gnome.ScreenSaver', '/org/gnome/ScreenSaver')
screensaver_iface = dbus.Interface(screensaver, dbus_interface='org.gnome.ScreenSaver')
is_locked = False

while True:
    is_active = screensaver_iface.GetActive()
    if is_locked != is_active:
        # System locked has been activated by something else than HSNP.
        is_locked = is_active
        if is_locked:
            print 'Sending lock signal.'
            ser.write('l')
        else:
           print 'Sending unlock signal.'
           ser.write('u')
    if ser.inWaiting() > 0:
        message = ser.read()
        if message == 'l':
            print 'Received lock signal.'
            screensaver_iface.Lock()
            is_locked = True
        elif message == 'u':
            print 'Received unlock signal.'
            screensaver_iface.SetActive(False)
            is_locked = False