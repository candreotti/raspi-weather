var express = require('express');
var router = express.Router();
var sensor = require('../model/sensor');
var db = require('../model/db');

/* GET available sensors. */
router.get('/sensors', function(req, res) {
    db.getSensors(res);
});

/* GET current sensor data. */
router.get('/current', function(req, res) {
    res.json(sensor.getCurrent());
});

/* GET last sensor data. */
router.get('/last/:sensor_mac', function(req, res) {
    var sensor_mac = req.params.sensor_mac;
    res.json(sensor.getLast(sensor_mac));
});

/* GET past 24h */
router.get('/past/24h/:sensor_mac', function(req, res) {
    // Callback...
    var sensor_mac = req.params.sensor_mac;
    db.getPast(24, sensor_mac, res);
});

/* GET past week */
router.get('/past/week/:sensor_mac', function(req, res) {
    var sensor_mac = req.params.sensor_mac;
    db.getPast(24*7, sensor_mac, res);
});

/* GET past month */
router.get('/past/month/:sensor_mac', function(req, res) {
    var sensor_mac = req.params.sensor_mac;
    db.getPast(24*30, sensor_mac, res);
});

/* GET yesterday vs today */
router.get('/compare/today/yesterday/:sensor_mac', function(req, res) {
    var sensor_mac = req.params.sensor_mac;
    db.getComparison('today', 'yesterday', sensor_mac, res);
});

module.exports = router;
