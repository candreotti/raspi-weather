import socket
import sys
from thread import *
import time
import json

HOST = ''
PORT = 3080

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print 'Socket created'

try:
    s.bind((HOST, PORT))
except socket.error as msg:
    print 'Bind failed. Error Code: ' + str(msg[0]) + ' Message ' + msg[1]
    sys.exit()

print 'Socket bind complete'

s.listen(10)
print 'Socket now listening on port ' + str(PORT)

def clientthread(conn):
    #conn.send('xxx')
    total_data=[]
    while True:
        data = conn.recv(4096)
        if not data:
            break
        total_data.append(data)
        #print 'Received: ' + data.decode('utf-8')

    conn.close()
    json_string = ''.join(total_data)
    print 'Total: ' + json_string
    sensor_data = json.loads(json_string)                       
    print 'TempC' + str(sensor_data["tempC"])                              
    degrees = sensor_data["tempC"]                                         
    hectopascals = sensor_data["pressure"] / 100                           
    altitude = sensor_data["altitudeM"]                                    
    humidity = sensor_data["humidity"]                                     
    timestamp = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime())       
    print 'Timestamp = %s'%timestamp + ', Temp = {0:0.3f} deg C'.format(degrees) + ', Pressure = {0:0.2f} hPa'.format(hectopascals) + ', Humidity = {0:0.2f} %'.format(humidity)

    import sqlite3
    import os
    dir_path = os.path.dirname(os.path.abspath(__file__))
    try:
        db = sqlite3.connect(os.path.join(dir_path, '../raspi-weather.db'))
        c = db.cursor()
        c.execute("""CREATE TABLE IF NOT EXISTS indoor(
            `id`            INTEGER PRIMARY KEY AUTOINCREMENT,
            `timestamp`     DATETIME,
            `temperature`   NUMERIC,
            `humidity`      NUMERIC,
            `pressure`      NUMERIC)""")
        db.commit()
	args = [timestamp, round(degrees, 2), int(humidity), round(hectopascals, 2)]
        c.execute('INSERT INTO indoor (timestamp, temperature, humidity, pressure) VALUES (?, ?, ?, ?)', args)
        db.commit()
        db.close()
        print("Done")
    except sqlite3.Error as err:
        f = open(os.path.join(dir_path, 'logger.log'), 'a')
        print(str(err))
        f.write(str(err))
        f.write('\n')
        f.close()

while 1:
    conn, addr = s.accept()
    print 'Connected with ' + addr[0] + ':' + str(addr[1])
    start_new_thread(clientthread, (conn,))

s.close()
