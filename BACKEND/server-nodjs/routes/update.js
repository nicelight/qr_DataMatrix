// routes/update.js
const express = require('express');
const path = require('path');
const multer = require('multer');
const updateController = require('../controllers/updateController');

const router = express.Router();

// Middleware проверки авторизации (для всех маршрутов админ-панели)
router.use((req, res, next) => {
  if (req.session.user) {
    next();
  } else {
    res.redirect('/login');
  }
});

// Дополнительный middleware для маршрутов управления оборудованием (только для admin)
router.use('/update', (req, res, next) => {
  if (req.session.user.role === 'admin') {
    next();
  } else {
    res.status(403).send("Доступ запрещен. Только для администраторов.");
  }
});

// Настройка multer для загрузки файла обновления в папку update
const storage = multer.diskStorage({
  destination: (req, file, cb) => {
    cb(null, path.join(__dirname, '../update'));
  },
  filename: (req, file, cb) => {
    cb(null, 'temp_' + Date.now() + '.zip');
  }
});
const upload = multer({ storage });

// Страница загрузки обновления
router.get('/', (req, res) => {
  res.render('update', { user: req.session.user });
});

// Обработка загрузки файла обновления через контроллер
router.post('/', upload.single('updateZip'), updateController.update);

module.exports = router;
