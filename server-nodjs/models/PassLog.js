const mongoose = require('mongoose');

const PassLogSchema = new mongoose.Schema({
  passId: { type: String, required: true },
  // status: 'activeEntrance', 'activeExit'
  status: { type: String, required: true },
  timestamp: { type: Date, default: Date.now },
  usageCount: { type: Number, default: 0 }
});

module.exports = mongoose.model('PassLog', PassLogSchema);
