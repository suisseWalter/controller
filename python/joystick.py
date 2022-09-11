from time import sleep
import pyvjoy
import serial as serial
import serial.tools.list_ports
# calibration values for all the joysticks,
# if thrust never goes to 0 increase first value
# if thrust reaches 0 to soon decrease first value
# if thrust never reaches 100% decrease second value
# if thrust reaches 100% to soon increase second value
# All values need to be between 0 and 1024.
# Only change the values in small increments(5).
calibration_values = [
    [80, 890], 
    [80, 880], 
    [80, 890]
    ]
def readline(ser:serial.Serial)->tuple[str,serial.Serial]:
    try:
        line=ser.readline().decode()
    except:
        print("exception")
        while(True):
            ports= serial.tools.list_ports.comports()
            print(ports)
            for port in ports:
                try:
                    ser = serial.Serial(port.device,115200,timeout=3)
                    sleep(0)
                    print(port)
                    if(ser.is_open):
                        print("found open port")
                        
                        line=ser.readline().decode()
                        print(line)
                        if "success" in line:
                            print("success was in line")
                            return line ,ser 
                except:
                        print("didn't read any infos from port")
                        
                    
                print("wasn't open")
    else:
        return line ,ser 

def scale_and_calibrate(axisid:int,value:int)->int :
    value=value - calibration_values[2-axisid][0]
    value=value*(32*1024/calibration_values[2-axisid][1])
    if value<0:
        value=0
    if value>32*1024:
        value=32*1024
    return int(value)
    




axis=["wAxisX","wAxisY","wAxisZ", "wThrottle","Rudder","wAileron","wAxisXRot","wAxisYRot","wAxisZRot","wSlider","wDial","wWheel","wAxisVX","wAxisVY","wAxisVZ","wAxisVBRX","wAxisVBRY","wAxisVBRZ" ]

ports= serial.tools.list_ports.comports()
joy= pyvjoy.VJoyDevice(1)
joy.reset_buttons()
joy.reset_povs()
joy.reset_data()
for port in ports:
    print(port)
ser = serial.Serial
while(True):
    line,ser=readline(ser)
    #print(type(line))
    if(line==''):
        continue
    sline= line.replace('<',' ').replace('>',' ').split()
    if(sline[0]=='-B'):
        butid=int( sline[1])
        joy.data.lButtons=butid
        #print(joy.data.lButtons)
        print("setbuttons",bin(butid)[2:].zfill(6))
        joy.update()
    elif(sline[0]=='success'):
        print("connected")
        pass
    elif(-1<0):
        axisid=-int( sline[0])
        axisvalue=int(sline[1])
        print("set axis (",axisid,",",axis[axisid-1],") to: ", scale_and_calibrate(axisid-1,axisvalue))
        setattr(joy.data,axis[axisid-1],scale_and_calibrate(axisid-1,axisvalue))
        joy.update()
    #print(sline)


