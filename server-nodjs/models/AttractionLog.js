const mongoose = require('mongoose');

const AttractionLogSchema = new mongoose.Schema({
  // Ссылка на оборудование-аттракцион (ID аттракциона из коллекции Equipment)
  equipment: { type: mongoose.Schema.Types.ObjectId, ref: 'Equipment', required: true },
  // ID пропуска, который воспользовался аттракционом
  passId: { type: String, required: true },
  // Время посещения
  timestamp: { type: Date, default: Date.now }
});

module.exports = mongoose.model('AttractionLog', AttractionLogSchema);
