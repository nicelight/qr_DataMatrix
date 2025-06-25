// Функция для добавления нового поля ввода
function addPassInputField() {
  const container = document.getElementById('passInputs');
  const newField = document.createElement('div');
  newField.className = 'form-group pass-input';
  newField.innerHTML = '<label>ID Пропуска</label><input type="text" name="passIds[]" required>';
  container.appendChild(newField);
  // Устанавливаем фокус на новом поле
  newField.querySelector('input').focus();
}

// Обработчик нажатия Enter в любом поле ввода ID пропуска
document.getElementById('passInputs').addEventListener('keydown', function(e) {
  if (e.key === "Enter") {
    e.preventDefault(); // предотвращаем отправку формы
    addPassInputField();
  }
});

// Обработчик для кнопки "Добавить поле"
document.getElementById('addFieldBtn').addEventListener('click', addPassInputField);

// Функция отправки запроса с массивом ID
async function sendRequest(direction) {
  const inputs = document.querySelectorAll('input[name="passIds[]"]');
  let passIds = [];
  inputs.forEach(input => {
    const value = input.value.trim();
    if (value !== '') {
      passIds.push(value);
    }
  });

  console.log('Отправляем passIds:', passIds);

  if (passIds.length === 0) {
    alert('Введите хотя бы один ID пропуска');
    return;
  }

  try {
    const response = await fetch('/admin/send', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ passIds, direction })
    });
    
    // Если запрос успешно обработан, обновляем текущую страницу
    if (response.ok) {
      window.location.reload();
    }
  } catch (error) {
    console.error('Ошибка при отправке запроса:', error);
  }
}
