const mongoose = require('mongoose');

const EquipmentSchema = new mongoose.Schema({
  name: { type: String, required: true },
  ip: { type: String, required: true },
  // type: 'Вход', 'Выход' для турникетов или 'аттракцион'
  type: { type: String, required: true }
});

module.exports = mongoose.model('Equipment', EquipmentSchema);
