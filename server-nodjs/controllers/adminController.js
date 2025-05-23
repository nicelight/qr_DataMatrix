const Equipment = require('../models/Equipment');
const AttractionLog = require('../models/AttractionLog');
const PassLog = require('../models/PassLog');
const axios = require('axios');

// Отображение главной страницы админ-панели
exports.dashboard = async (req, res) => {
  try {
    // Получаем статистику пропусков
    const activeEntranceCount = await PassLog.countDocuments({ status: 'activeEntrance' });
    const activeExitCount = await PassLog.countDocuments({ status: 'activeExit' });
    
    // Агрегация по оборудованию типа "аттракцион" с использованием $lookup для подсчёта посещений.
	const attractionData = await Equipment.aggregate([
	  { $match: { type: 'аттракцион' } },
	  { $lookup: {
		  from: 'attractionlogs',
		  localField: '_id',
		  foreignField: 'equipment',
		  as: 'logs'
	  }},
	  { $addFields: { count: { $size: '$logs' } } },
	  { $project: { equipmentName: '$name', count: 1 } },
	  { $sort: { equipmentName: 1 } }   // сортировка по имени по возрастанию
	]);
    
    res.render('dashboard', { 
      activeEntranceCount, 
      activeExitCount, 
      attractionData,  
      user: req.session.user
    });
  } catch (error) {
    console.error(error);
    res.status(500).send('Ошибка при загрузке админ-панели');
  }
};

// Отправка запроса на все турникеты нужного типа (Вход или Выход)
exports.sendManualPost = async (req, res) => {
  const { passIds, direction } = req.body; // Ожидается массив passIds и направление "Вход" или "Выход"
  let newStatus;
  if (direction === 'Вход') {
    newStatus = 'activeEntrance';
  } else if (direction === 'Выход') {
    newStatus = 'activeExit';
  } else {
    return res.status(400).send('Неверное направление');
  }
       
  // Обновляем записи в базе для каждого passId
  await Promise.all(passIds.map(id => {
    return PassLog.findOneAndUpdate(
      { passId: id },
      { status: newStatus },
      { upsert: true, new: true }
    );
  })); 
  
  // Немедленное обновление текущей страницы (редирект на предыдущий URL или /admin)
  res.redirect(req.get('referer') || '/admin');
  
};
// Получение списка оборудования
exports.equipmentList = async (req, res) => {
  try {
    const equipments = await Equipment.find({});
    res.render('equipment', { equipments, user: req.session.user });
  } catch (error) {
    console.error(error);
    res.status(500).send('Ошибка при получении списка оборудования');
  }
};

// Добавление нового оборудования
exports.addEquipment = async (req, res) => {
  try {
    const { name, ip, type } = req.body;
    const equipment = new Equipment({ name, ip, type });
    await equipment.save();
    res.redirect('/admin/equipment');
  } catch (error) {
    console.error(error);
    res.status(500).send('Ошибка при добавлении оборудования');
  }
};

// Отображение формы редактирования оборудования
exports.getEditEquipment = async (req, res) => {
  try {
    const equipment = await Equipment.findById(req.params.id);
    if (!equipment) return res.status(404).send('Оборудование не найдено');
    res.render('equipment_edit', { equipment, user: req.session.user });
  } catch (error) {
    console.error(error);
    res.status(500).send('Ошибка при получении данных оборудования');
  }
};

// Сохранение изменений оборудования
exports.updateEquipment = async (req, res) => {
  try {
    const { name, ip, type } = req.body;
    await Equipment.findByIdAndUpdate(req.params.id, { name, ip, type });
    res.redirect('/admin/equipment');
  } catch (error) {
    console.error(error);
    res.status(500).send('Ошибка при обновлении оборудования');
  }
};

// Удаление оборудования по ID
exports.deleteEquipment = async (req, res) => {
  try {
    const id = req.params.id;

    // Удаляем само оборудование
    await Equipment.findByIdAndDelete(id);

    // Удаляем все записи посещений этого аттракциона
    await AttractionLog.deleteMany({ equipment: id });

    return res.redirect('/admin/equipment');
  } catch (error) {
    console.error('Ошибка при удалении оборудования и логов аттракциона:', error);
    return res.status(500).send('Ошибка при удалении оборудования');
  }
};

// Новый метод: Получение списка активных пропусков по типу
exports.getPassLogs = async (req, res) => {
  try {
    // Получаем тип записей, например, "activeEntrance" или "activeExit" из параметров запроса
    const passType = req.query.type || 'activeEntrance';
    // Находим записи с нужным статусом и сортируем по дате (новейшие в начале)
    const passLogs = await PassLog.find({ status: passType }).sort({ timestamp: -1 });
    res.render('passlog', { passLogs, passType, user: req.session.user });
  } catch (error) {
    console.error('Ошибка при получении записей PassLog:', error);
    res.status(500).send('Ошибка при получении записей');
  }
};

// Новый метод: Массовое удаление выбранных записей PassLog с удалением кода с турникетов
exports.deletePassLogs = async (req, res) => {
  try {
    const passIds = req.body.passIds;
    if (!passIds || passIds.length === 0) {
      return res.redirect('/admin/passlog');
    }
    
    // Сохраняем тип записей для редиректа
    const passType = req.query.type || 'activeEntrance';
           
	// Удаляем записи из базы данных
	await PassLog.deleteMany({ passId: { $in: passIds } });  
	
    // Отправляем немедленный ответ (редирект) клиенту
    res.redirect('/admin');

    
  } catch (error) {
    console.error('Ошибка при удалении записей PassLog:', error);
    if (!res.headersSent) {
      res.status(500).send('Ошибка при удалении записей');
    }
  }
};