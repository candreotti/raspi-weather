#!/usr/bin/python

import sys
import os
import sqlite3

dir_path = os.path.dirname(os.path.abspath(__file__))

try:
    sensor_ip = sys.argv[1]
    db = sqlite3.connect(os.path.join(dir_path, '../raspi-weather.db'))
    c = db.cursor()
    c.execute('SELECT * FROM indoor WHERE sensor_ip = ? ORDER BY id DESC LIMIT 1', (sensor_ip,))
    row = c.fetchone()
    timestamp = row[1]
    temperature = row[2]
    humidity = row[3]
    hectopascals = row[4]
    print timestamp + '\n' + '{0:0.1f}\n{1}\n{2:0.2f}'.format(temperature, int(humidity), hectopascals)
except RuntimeError as e:
    print 'error\n{0}'.format(e)
except:
    print 'error\nFailed to retrieve last measure'