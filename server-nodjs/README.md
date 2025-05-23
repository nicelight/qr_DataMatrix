# child-playground-manager
**Child-Playground Manager** – это веб-система для управления детскими развлекательными площадками, включающая управление турникетами, аттракционами. Проект позволяет:

- **Администратору:**
  - Управлять списком оборудования (турникеты, аттракционы)
  - Просматривать статистику по использованию пропусков и аттракционов (с графиками).

- **API:**
  - Обеспечивает связь с оборудованием. Запросы от турникетов и аттракционов обрабатываются согласно их типу и переданным кодам.
  - Для оборудования типа "аттракцион" каждое посещение фиксируется отдельно.
  - При процедуре INIT дополнительные проверки не требуются.

## Технологии

- **Node.js** с использованием Express
- **MongoDB** для хранения данных
- **Mongoose** – ORM для MongoDB
- **EJS** – шаблонизатор для генерации HTML
- **Axios** – для отправки HTTP-запросов к оборудованию
- **Chart.js** – для построения графиков

## Установка и запуск

### Требования
- [Node.js](https://nodejs.org/) (рекомендуется последняя LTS-версия)
- [MongoDB](https://www.mongodb.com/try/download/community) (локально или в облаке)

### Инструкции

1. **Клонирование репозитория:**

   git clone https://github.com/yourusername/playground-manager.git
   cd playground-manager  

2. **Установка зависимостей:**

	npm install
	
3. **Настройка базы данных:**
Создайте файл конфигурации, например, config/db.js, и укажите строку подключения к вашей базе данных MongoDB.
При необходимости создайте файл .env (например, строку подключения, сессии и т.д.).

4. **Запуск приложения:**

Для разработки:
	npm run dev
Для запуска в продакшене:
	npm start
	
5. **Доступ к интерфейсу:**

Админка: http://localhost/admin
API (оборудование): http://localhost/api/callback