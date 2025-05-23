const express = require('express');
const router = express.Router();
const adminController = require('../controllers/adminController');

// Middleware проверки авторизации (для всех маршрутов админ-панели)
router.use((req, res, next) => {
  if (req.session.user) {
    next();
  } else {
    res.redirect('/login');
  }
});

// Дополнительный middleware для маршрутов управления оборудованием (только для admin)
router.use('/equipment', (req, res, next) => {
  if (req.session.user.role === 'admin') {
    next();
  } else {
    res.status(403).send("Доступ запрещен. Только для администраторов.");
  }
});

// Основные маршруты админ-панели
router.get('/', adminController.dashboard);
router.post('/send', adminController.sendManualPost);

// Маршруты управления оборудованием
router.get('/equipment', adminController.equipmentList);
router.post('/equipment', adminController.addEquipment);
router.get('/equipment/edit/:id', adminController.getEditEquipment);
router.post('/equipment/edit/:id', adminController.updateEquipment);
router.get('/equipment/delete/:id', adminController.deleteEquipment);
router.get('/passlog', adminController.getPassLogs);
router.post('/passlog/delete', adminController.deletePassLogs);

module.exports = router;
