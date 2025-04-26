const char *FileStorage_html =
    R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>Pico File Manager</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <style>
    body {
      font-family: 'Arial', sans-serif;
      background: #e0e5ec;
      margin: 0;
      padding: 20px;
      display: flex;
      flex-direction: column;
      align-items: center;
    }
    .card {
      background: #e0e5ec;
      border-radius: 20px;
      box-shadow: 9px 9px 16px #bec3c9, -9px -9px 16px #ffffff;
      padding: 20px;
      max-width: 800px;
      width: 100%;
      margin-bottom: 20px;
    }
    .button {
      background: #e0e5ec;
      border: none;
      border-radius: 12px;
      box-shadow: 5px 5px 10px #bec3c9, -5px -5px 10px #ffffff;
      padding: 10px 20px;
      margin: 10px;
      cursor: pointer;
      font-weight: bold;
    }
    .button:hover {
      background: #d1d9e6;
    }
    table {
      width: 100%;
      border-collapse: collapse;
      margin-top: 10px;
    }
    th, td {
      padding: 10px;
      text-align: left;
    }
    th {
      border-bottom: 2px solid #ccc;
    }
    .danger {
      background: #ff4d4d;
      color: white;
    }
    .danger:hover {
      background: #e03b3b;
    }
    input[type="file"] {
      display: none;
    }
    label.upload-label {
      display: inline-block;
      padding: 10px 20px;
      border-radius: 12px;
      background: #e0e5ec;
      box-shadow: 5px 5px 10px #bec3c9, -5px -5px 10px #ffffff;
      cursor: pointer;
      font-weight: bold;
    }
    label.upload-label:hover {
      background: #d1d9e6;
    }
    .path-input {
      margin-top: 10px;
      display: flex;
      align-items: center;
      gap: 10px;
    }
    .path-input input {
      border-radius: 12px;
      border: 1px solid #ccc;
      padding: 5px 10px;
    }
  </style>
</head>
<body>

  <div class="card">
    <h2>📂 Pico File Manager</h2>

    <div class="path-input">
      <strong>Path:</strong> <span id="currentPath">/uploads</span>
      <input type="text" id="newPath" placeholder="/uploads" style="flex-grow: 1;">
      <button class="button" onclick="changePath()">Change Path</button>
    </div>

    <div style="margin-bottom: 10px;">
      <label for="fileUpload" class="upload-label">Upload File</label>
      <input type="file" id="fileUpload" onchange="uploadFile()">
      <button class="button danger" onclick="formatFlash()">Format Storage</button>
    </div>

    <table>
      <thead>
        <tr>
          <th>Filename</th>
          <th>Size</th>
          <th>Actions</th>
        </tr>
      </thead>
      <tbody id="fileTableBody">
        <!-- Files will populate here -->
      </tbody>
    </table>
  </div>

  <script>
    let currentPath = '/uploads';

async function listFiles() {
  try {
    const res = await fetch('/api/v1/ls' + currentPath);

    if (!res.ok) {
      if (res.status === 404) {
        const tbody = document.getElementById('fileTableBody');
        tbody.innerHTML = '<tr><td colspan="3" style="text-align:center;">No files found.</td></tr>';
        return;
      } else {
        throw new Error(`Failed to list files: ${res.status}`);
      }
    }

    const files = await res.json();

    const tbody = document.getElementById('fileTableBody');
    tbody.innerHTML = '';

    if (!Array.isArray(files) || files.length === 0) {
      const row = document.createElement('tr');
      row.innerHTML = '<td colspan="3" style="text-align:center;">No files found.</td>';
      tbody.appendChild(row);
      return;
    }

    files.forEach(file => {
      if (!file || !file.name || typeof file.size !== 'number') {
        return; // skip invalid
      }
      const row = document.createElement('tr');
      const sizeKB = (file.size / 1024).toFixed(1);
      row.innerHTML = `
        <td>${file.name}</td>
        <td>${sizeKB} KB</td>
        <td>
          <button class="button" onclick="previewFile('${file.name}')">Preview</button>
          <button class="button danger" onclick="deleteFile('${file.name}')">Delete</button>
        </td>
      `;
      tbody.appendChild(row);
    });

  } catch (err) {
    console.error('Error listing files:', err);
    const tbody = document.getElementById('fileTableBody');
    tbody.innerHTML = '<tr><td colspan="3" style="text-align:center;">Failed to load files</td></tr>';
  }
}

    async function deleteFile(filename) {
      if (confirm(`Are you sure you want to delete ${filename}?`)) {
        const fullPath = currentPath + '/' + filename;
        await fetch('/api/v1/files' + encodeURIComponent(fullPath), { method: 'DELETE' });
        await listFiles();
      }
    }

    async function uploadFile() {
      const fileInput = document.getElementById('fileUpload');
      const file = fileInput.files[0];
      if (!file) return;

      const formData = new FormData();
      formData.append('file', file);

      await fetch('/api/v1/upload', {
        method: 'POST',
        body: formData
      });

      fileInput.value = '';
      await listFiles();
    }

    async function formatFlash() {
      if (confirm('⚠️ Format storage? This will erase all files!')) {
        await fetch('/api/v1/format_storage', { method: 'POST' });
        currentPath = '/uploads';
        document.getElementById('currentPath').textContent = currentPath;
        document.getElementById('newPath').value = currentPath;
        await listFiles();
      }
    }

    function changePath() {
      let newPath = document.getElementById('newPath').value.trim();
      if (!newPath.startsWith('/')) {
        newPath = '/' + newPath;
      }
      currentPath = newPath;
      document.getElementById('currentPath').textContent = currentPath;
      listFiles();
    }

    function previewFile(filename) {
      const url = currentPath + '/' + encodeURIComponent(filename);
      window.open(url, '_blank');
    }

    listFiles();
  </script>

</body>
</html>
)rawliteral";
