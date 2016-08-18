# Raspberry Pi weather station

(Original project: https://github.com/ofalvai/raspi-weather)

BME280 temperature/humidity/pressure sensor logger and browser dashboard for the Raspberry Pi. Based on [Adafruit's BME280 Python library](https://github.com/adafruit/Adafruit_Python_BME280).

![Screenshot](/public/images/screenshot.png?raw=true)

# Features

- Measure and store temperature, humidity and pressure periodically (via cron and sqlite)
- Responsive web dashboard
- Display current temperature, humidity and pressure
- Display logged temperature, humidity and pressure graphs
- Other weather info via [Forecast.io](http://forecast.io)

# Installation

```
sudo apt-get install sqlite3
wget http://node-arm.herokuapp.com/node_latest_armhf.deb
sudo dpkg -i node_latest_armhf.deb
git clone https://github.com/dventurino/raspi-weather.git
cd raspi-weather
npm install
```

LOCAL SENSOR MODE:

Install Adafruit's BME280 Python library [according to their instructions](https://github.com/adafruit/Adafruit_Python_BME280.git).

Edit your sudo crontab with `sudo crontab -e` (yes, it needs to run as root to access GPIO, use at your own risk), and add this line:

```
*/30 * * * * /usr/bin/python /home/pi/raspi-weather/sensor_scripts/logger.py
```

...assuming you want to take measurements every 30 minutes, and cloned into `/home/pi`.

Connect your BME280 sensor to the Pi, copy Adafruit_BME280.py in sensors and set the correct BME280 default address.

You can test both scripts by running `sudo sensor_scripts/current.py` and `sudo sensor_scripts/logger.py`. The latter will create the sqlite database file in the project root and log the first measurement.

REMOTE SENSORS MODE:

Edit your sudo crontab with `sudo crontab -e` and add this line:

```
@reboot . /home/pi/raspi-weather/sensor_scripts/server_socket.sh
```

This is to start the socket listening process on startup, il will listen for sensors messages on port 3080 and it will save received measures in the DB.


CONFIGURATION:

You can further tweak the frontend settings in `public/javascripts/app.js`, like:

- operation mode
- temperature unit
- Forecast.io API key and location for outside weather info
- chart options

# Usage

Start the server (it needs to run as root for the same reason as the cronjob: to aceess GPIO):

```
sudo nohup node app.js &
```

...or you can install [forever](https://github.com/foreverjs/forever) to keep it _always_ running and then `sudo forever start app.js`

...or you can add its init script to make it start at boot.

The server runs on port 3000, so visit for example `http://192.168.0.100:3000`

To keep the server _always_ running even during reboot you can install [forever-service](https://github.com/zapty/forever-service) ant then:

```
cd <raspi-weather directory>
sudo forever-service install raspi-weather
sudo update-rc.d raspi-weather defaults
```

# Future ideas

- Select custom date ranges to display on the graphs
- Average graphs
- Dynamic favicon (like Google Calendar)

Feel free to fork and send me pull requests :)
