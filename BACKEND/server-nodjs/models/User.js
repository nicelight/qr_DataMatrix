const mongoose = require('mongoose');

const UserSchema = new mongoose.Schema({
  username: { type: String, required: true, unique: true },
  password: { type: String, required: true },
  role: { type: String, enum: ['read', 'admin'], default: 'read' }
});

module.exports = mongoose.model('User', UserSchema);
