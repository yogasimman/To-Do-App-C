document.addEventListener("DOMContentLoaded", function () {
    // Load tasks from JSON file using AJAX
    loadTasks();

    // Add event listener for opening the modal
    document.getElementById("open-modal").addEventListener("click", function () {
        document.getElementById("task-modal").style.display = "flex";
    });

    // Add event listener for closing the modal
    document.getElementById("close-modal").addEventListener("click", function () {
        document.getElementById("task-modal").style.display = "none";
    });

    // Add event listener for form submission
    document.getElementById("task-form").addEventListener("submit", function (event) {
        event.preventDefault();
        const title = document.getElementById("task-title").value.trim();
        const content = document.getElementById("task-content").value.trim();
        
        if (title && content) {
            // If both title and content are not empty, send a POST request to the server
            const taskData = {
                title: title,
                content: content
            };

            fetch('/', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(taskData)
            })
            .then(response => response.json())
            .then(data => {
                console.log('Success:', data);
                // Clear form and close modal
                console.log(data);
                document.getElementById("task-form").reset();
                document.getElementById("task-modal").style.display = "none";
                // Reload the task list after adding a new task
                loadTasks(); 
            })
            .catch((error) => {
                console.error('Error:', error);
            });
        } else {
            alert('Please fill out both the title and content fields.');
        }
    });
});

function loadTasks() {
    const xhr = new XMLHttpRequest();
    xhr.open('GET', 'tasks.json?timestamp=' + new Date().getTime(), true); // Append timestamp to avoid caching
    xhr.onload = function () {
        if (xhr.status === 200) {
            try {
                const tasks = JSON.parse(xhr.responseText);
                const taskList = document.getElementById("task-list");
                taskList.innerHTML = ''; // Clear the task list
                tasks.forEach(task => addTask(task));
            } catch (error) {
                console.error('Error parsing JSON:', error);
            }
        } else {
            console.error('Failed to load tasks:', xhr.statusText);
        }
    };
    xhr.send();
}


function addTask(task) {
    const taskList = document.getElementById("task-list");

    const taskElement = document.createElement("div");
    taskElement.classList.add("task");

    const taskTitle = document.createElement("h2");
    taskTitle.classList.add("task-title");
    taskTitle.textContent = task.title;

    const hr = document.createElement("hr");

    const taskContent = document.createElement("p");
    taskContent.classList.add("task-content");
    taskContent.textContent = task.content;

    const deleteButton = document.createElement("button");
    deleteButton.classList.add("delete-btn");
    deleteButton.innerHTML = '<i class="fas fa-trash"></i>';
    deleteButton.addEventListener("click", function () {
        fetch('/', {
            method: 'DELETE',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ title: task.title }) // Assuming title is unique
        })
        .then(response => response.json())
        .then(data => {
            console.log('Task deleted:', data);
            // Reload the task list after deleting a task
            console.log(data);
            loadTasks(); 
        })
        .catch((error) => {
            console.error('Error:', error);
        });
    });

    taskElement.appendChild(taskTitle);
    taskElement.appendChild(hr);
    taskElement.appendChild(taskContent);
    taskElement.appendChild(deleteButton);

    taskList.appendChild(taskElement);
}
