const express = require('express');
const router = express.Router();
const apiController = require('../controllers/apiController');

// Маршрут для API-запросов от оборудования по адресу /api/callback
router.post('/callback', apiController.api);

module.exports = router;