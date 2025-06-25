// update/update.js
const fs = require('fs');
const path = require('path');
const AdmZip = require('adm-zip');
const semver = require('semver');
const fse = require('fs-extra');

// Определяем корневую папку проекта (на уровень выше папки update)
const projectRoot = path.resolve(__dirname, '..');
// Чтение текущей версии из package.json, расположенного в корне проекта
const packageJson = require(path.join(projectRoot, 'package.json'));
const currentVersion = packageJson.version;

/**
 * Функция создания резервной копии проекта, исключая папки node_modules и update.
 * Резервная копия создается в папке, имя которой имеет формат backup_версия.
 */
function createBackup() {

  const backupPath = path.join(projectRoot, '..', `backup_${currentVersion}`);
  console.log('Создание резервной копии проекта в:', backupPath);

  try {
    fse.copySync(projectRoot, backupPath, {
      filter: (src) => {
        // Получаем относительный путь от корня проекта
        const relativePath = path.relative(projectRoot, src);
        // Если путь начинается с node_modules или update, то не копируем
        if (relativePath.startsWith('node_modules') || relativePath.startsWith('update')) {
          return false;
        }
        return true;
      }
    });
    console.log('Резервная копия успешно создана:', backupPath);
    return true;
  } catch (err) {
    console.error('Ошибка при создании резервной копии:', err);
    throw err;
  }
}

/**
 * Функция проверки и применения обновления.
 * Если в папке update находится архив update.zip и его версия новее текущей, 
 * создаётся резервная копия проекта (без node_modules и update) и обновление применяется.
 */
function checkAndApplyUpdate() {
  return new Promise((resolve, reject) => {
    // Путь к архиву обновления в папке update
    const updateDir = path.join(projectRoot, 'update');
    const updateZipPath = path.join(updateDir, 'update.zip');

    if (!fs.existsSync(updateZipPath)) {
      console.log('Файл обновления не найден.');
      return resolve(false);
    }

    console.log('Обнаружен файл обновления. Начинается процесс обновления.');

    // Распаковка архива во временную папку в корне проекта
    const extractPath = path.join(projectRoot, 'update_temp');
    try {
      const zip = new AdmZip(updateZipPath);
      zip.extractAllTo(extractPath, true);
      console.log('Архив успешно распакован в:', extractPath);
    } catch (err) {
      console.error('Ошибка распаковки архива:', err);
      return reject(err);
    }

    // Если вы храните версию в package.json, можно читать обновленный package.json из архива
    const packageFilePath = path.join(extractPath, 'package.json');
    if (!fs.existsSync(packageFilePath)) {
      console.error('Файл package.json не найден в обновлении.');
      cleanup(updateZipPath, extractPath);
      return reject(new Error('package.json not found'));
    }

    let updatePackage;
    try {
      updatePackage = require(packageFilePath);
    } catch (err) {
      console.error('Ошибка чтения package.json из обновления:', err);
      cleanup(updateZipPath, extractPath);
      return reject(err);
    }

    const updateVersion = updatePackage.version;
    if (!semver.valid(updateVersion)) {
      console.error('Некорректный формат версии обновления.');
      cleanup(updateZipPath, extractPath);
      return reject(new Error('Invalid version format'));
    }

    // Сравнение версий
    if (semver.gt(updateVersion, currentVersion)) {
      console.log(`Применяется обновление: ${currentVersion} → ${updateVersion}`);

      // Создание резервной копии всего проекта, исключая node_modules и update
      try {
        createBackup();
      } catch (backupErr) {
        cleanup(updateZipPath, extractPath);
        return reject(backupErr);
      }

      // Применение обновления: копирование файлов из распакованного архива в корневую директорию проекта
      try {
        fse.copySync(extractPath, projectRoot, { overwrite: true });
        console.log('Обновление успешно применено.');
      } catch (err) {
        console.error('Ошибка при применении обновления:', err);
        cleanup(updateZipPath, extractPath);
        return reject(err);
      }
    } else {
      console.log('Обновление не применяется: версия обновления не новее текущей.');
    }

    // Очистка временных файлов
    cleanup(updateZipPath, extractPath);
    resolve(semver.gt(updateVersion, currentVersion));
  });
}

/**
 * Функция очистки временных файлов: удаляет архив обновления и временную папку с распакованными данными.
 */
function cleanup(zipPath, tempDir) {
  try {
    if (fs.existsSync(zipPath)) {
      fs.unlinkSync(zipPath);
      console.log('Удалён архив обновления:', zipPath);
    }
    if (fs.existsSync(tempDir)) {
      fse.removeSync(tempDir);
      console.log('Удалена временная папка:', tempDir);
    }
  } catch (err) {
    console.error('Ошибка при очистке временных файлов:', err);
  }
}

module.exports = { checkAndApplyUpdate };
