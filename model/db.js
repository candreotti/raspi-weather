var sqlite3 = require('sqlite3');

var db = {
    DB_PATH: './raspi-weather.db',
    connection: null,

    connect: function() {
        db.connection = new sqlite3.Database(db.DB_PATH);
    },

    errorHandler: function(err, res) {
        res.json({
            success: false,
            error: err.toString()
        });
    },

    getSensors: function(res) {
        db.connect();

        var stmt = db.connection.prepare(
            'SELECT * FROM sensors',
            function(err) {
                if(err) {
                    db.errorHandler(err, res);
                }
            }
        );
        stmt.all(function(err, rows) {
            if(err) {
                db.errorHandler(err, res);
            } else {
                res.json({
                    success: true,
                    data: rows
                });
            }
        });
    },

    getPast: function(hours, sensor_mac, res) {
        db.connect();

        var hourString = -hours + ' hours';
        var stmt = db.connection.prepare(
            'SELECT timestamp, temperature, humidity, pressure FROM indoor WHERE timestamp >= datetime(?, ?, ?) AND sensor_mac = ?',
            function(err) {
                if(err) {
                    db.errorHandler(err, res);
                }
            }
        );
        stmt.all(['now', 'localtime', hourString, sensor_mac], function(err, rows) {
            if(err) {
                db.errorHandler(err, res);
            } else {
                res.json({
                    success: true,
                    data: rows
                });
            }
        });
    },

    getComparison: function(firstType, secondType, sensor_mac, res) {
        db.connect();

        var result = {
            first: {
                type: firstType,
                data: []
            },
            second: {
                type: secondType,
                data: []
            }
        };

        db.connection.serialize(function() {
            if(firstType === 'today') {
                var stmt = db.connection.prepare(
                    'SELECT timestamp, temperature, humidity, pressure FROM indoor WHERE timestamp >= datetime(?, ?, ?) AND sensor_mac = ?',
                    function(err) {
                        if(err) {
                            db.errorHandler(err, res);
                        }
                    }
                );
                stmt.all(['now', 'localtime', 'start of day', sensor_mac], function(err, rows) {
                    if(err) {
                        db.errorHandler(err, res);
                    } else {
                        result.first.data = rows;
                        // The result is not yet ready,
                        // we'll return both in the second query callback
                        // (the queries are serialized)
                    }
                });
            }

            if(secondType === 'yesterday') {
                var stmt = db.connection.prepare(
                    'SELECT timestamp, temperature, humidity, pressure\
                    FROM indoor WHERE timestamp >= datetime($now, $timezone, $start, $minus)\
                    AND timestamp <= datetime($now, $timezone, $start) AND sensor_mac = $sensorMAC',
                    function(err) {
                        if(err) {
                            db.errorHandler(err, res);
                        }
                    }
                );
                stmt.all({
                    $now: 'now',
                    $timezone: 'localtime',
                    $start: 'start of day',
                    $minus: '-1 day',
                    $sensorMAC: sensor_mac
                }, function(err, rows) {
                    if(err) {
                        db.errorHandler(err, res);
                    } else {
                        result.second.data = rows;
                        result.success = true;
                        res.json(result);
                    }
                });
            }
        });
    }
};

module.exports = db;
