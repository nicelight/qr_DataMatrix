const User = require('../models/User');
const bcrypt = require('bcrypt');

exports.loginPage = (req, res) => {
  res.render('login', { error: null });
};

exports.login = async (req, res) => {
  const { username, password } = req.body;

  try {
    const user = await User.findOne({ username });

    if (!user) {
      return res.render('login', { error: 'Неверные логин или пароль' });
    }

    const isMatch = await bcrypt.compare(password, user.password);

    if (isMatch) {
      req.session.user = user;
      res.redirect('/admin');
    } else {
      res.render('login', { error: 'Неверные логин или пароль' });
    }
  } catch (err) {
    console.error(err);
    res.render('login', { error: 'Ошибка сервера' });
  }
};

exports.logout = (req, res) => {
  req.session.destroy();
  res.redirect('/login');
};
