// controllers/updateController.js
const path = require('path');
const fs = require('fs');
const updateModule = require('../update/update');

const axios = require('axios');

exports.update = async (req, res) => {
  try {
    if (!req.file) {
      return res.status(400).send('Файл не загружен.');
    }
    
    // Определяем путь к папке update (на уровень выше контроллера)
    const updateDir = path.join(__dirname, '../update');
    const newFilePath = path.join(updateDir, 'update.zip');
    
    // Если файл update.zip уже существует, удаляем его
    if (fs.existsSync(newFilePath)) {
      await fs.promises.unlink(newFilePath);
    }
    
    // Переименовываем временный файл в update.zip
    await fs.promises.rename(req.file.path, newFilePath);
    
    // Вызываем функцию проверки и применения обновления
    const updated = await updateModule.checkAndApplyUpdate();
    
    if (updated) {
      res.send('Обновление успешно применено.');
    } else {
      res.send('Обновление не требуется или версия обновления не новее текущей.');
    }
  } catch (err) {
    console.error('Ошибка при обновлении:', err);
    res.status(500).send('Ошибка при обновлении.');
  }
};
