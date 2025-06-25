const express = require('express');
const session = require('express-session');
const bodyParser = require('body-parser');
const path = require('path');
const mongoose = require('mongoose');
const dbConfig = require('./config/db');

const apiRoutes = require('./routes/api');
const adminRoutes = require('./routes/admin');
const authRoutes = require('./routes/auth');
const updateRoutes = require('./routes/update'); 
const User = require('./models/User');

const app = express();
const favicon = require('serve-favicon');
const bcrypt = require('bcrypt');
const fs = require('fs');

// Подключение к MongoDB
mongoose.connect(dbConfig.mongoURI)
  .then(() => console.log('MongoDB connected'))
  .catch(err => console.error('MongoDB connection error:', err));

// Настройка сессий
app.use(session({
  secret: 'some-secret-key',
  resave: false,
  saveUninitialized: true
}));

mongoose.connection.once('open', async () => {
  const userCount = await User.countDocuments();
  if (userCount === 0) {
    const hashedPassword = await bcrypt.hash('admin', 10);
    await User.create({
      username: 'admin',
      password: hashedPassword,
      role: 'admin'
    });
    const hashedUserPassword = await bcrypt.hash('12345', 10);
    await User.create({
      username: 'user',
      password: hashedUserPassword,
      role: 'read'
    });
    console.log('Default users created');
  }
});

// Настройка шаблонизатора EJS и папки views
app.set('view engine', 'ejs');
app.set('views', path.join(__dirname, 'views'));

app.use(favicon(path.join(__dirname, 'public', 'favicon.ico')));

// Подключение статических файлов
app.use(express.static(path.join(__dirname, 'public')));

// Настройка body-parser
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));

// Главная страница: перенаправление на админку, если пользователь авторизован, иначе на страницу логина
app.get('/', (req, res) => {
  if (req.session.user) {
    res.redirect('/admin');
  } else {
    res.redirect('/login');
  }
});

// Маршруты авторизации
app.use('/', authRoutes);

// API-роуты для оборудования
app.use('/api', apiRoutes);

// Маршруты админ-панели (доступ только после авторизации)
app.use('/admin', adminRoutes);

// Маршруты обновления
app.use('/update', updateRoutes);

const PORT = process.env.PORT || 80;
app.listen(PORT, () => {
  console.log(`Server started on port ${PORT}`);
});
