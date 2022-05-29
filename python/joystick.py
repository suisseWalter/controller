from time import sleep
import pyvjoy
import serial as serial
import serial.tools.list_ports


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
        print("set axis (",axisid,",",axis[axisid-1],") to: ", axisvalue)
        setattr(joy.data,axis[axisid-1],32*axisvalue)
        joy.update()
    #print(sline)


