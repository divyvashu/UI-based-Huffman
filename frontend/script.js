const dropZone = document.getElementById('drop-zone');
const fileInput = document.getElementById('file-input');
const fileInfo = document.getElementById('file-info');
const fileName = document.getElementById('file-name');
const fileSize = document.getElementById('file-size');
const removeBtn = document.getElementById('remove-file');
const actionBtn = document.getElementById('action-btn');
const btnCompress = document.getElementById('btn-compress');
const btnDecompress = document.getElementById('btn-decompress');
const resultsCard = document.getElementById('results-card');

let currentFile = null;
let mode = 'compress'; // 'compress' or 'decompress'

// Mode Switching
btnCompress.addEventListener('click', () => setMode('compress'));
btnDecompress.addEventListener('click', () => setMode('decompress'));

function setMode(newMode) {
    mode = newMode;
    btnCompress.classList.toggle('active', mode === 'compress');
    btnDecompress.classList.toggle('active', mode === 'decompress');
    actionBtn.textContent = mode === 'compress' ? 'Compress File' : 'Decompress File';
    resultsCard.style.display = 'none';
}

// Drag & Drop
dropZone.addEventListener('click', () => fileInput.click());

dropZone.addEventListener('dragover', (e) => {
    e.preventDefault();
    dropZone.classList.add('dragover');
});

dropZone.addEventListener('dragleave', () => {
    dropZone.classList.remove('dragover');
});

dropZone.addEventListener('drop', (e) => {
    e.preventDefault();
    dropZone.classList.remove('dragover');
    if (e.dataTransfer.files.length) {
        handleFile(e.dataTransfer.files[0]);
    }
});

fileInput.addEventListener('change', () => {
    if (fileInput.files.length) {
        handleFile(fileInput.files[0]);
    }
});

function handleFile(file) {
    currentFile = file;
    fileName.textContent = file.name;
    fileSize.textContent = formatBytes(file.size);
    fileInfo.style.display = 'flex';
    dropZone.style.display = 'none';
    actionBtn.disabled = false;
    resultsCard.style.display = 'none';
}

removeBtn.addEventListener('click', () => {
    currentFile = null;
    fileInput.value = '';
    fileInfo.style.display = 'none';
    dropZone.style.display = 'block';
    actionBtn.disabled = true;
    resultsCard.style.display = 'none';
});

// API Communication
actionBtn.addEventListener('click', async () => {
    if (!currentFile) return;

    actionBtn.disabled = true;
    actionBtn.textContent = 'Processing...';

    const formData = new FormData();
    formData.append('file', currentFile);

    // Use relative paths for API
    const BASE_URL = "https://huffman.up.railway.app";
    
    const endpoint = mode === 'compress'
        ? `${BASE_URL}/api/compress`
        : `${BASE_URL}/api/decompress`;


    try {
        const response = await fetch(endpoint, {
            method: 'POST',
            body: formData
        });

        if (!response.ok) throw new Error('Processing failed');

        const data = await response.json();
        showResults(data);

    } catch (error) {
        alert('Error: ' + error.message);
    } finally {
        actionBtn.disabled = false;
        actionBtn.textContent = mode === 'compress' ? 'Compress File' : 'Decompress File';
    }
});

function showResults(data) {
    resultsCard.style.display = 'block';

    const label1 = document.getElementById('label-1');
    const val1 = document.getElementById('val-1');
    const label2 = document.getElementById('label-2');
    const val2 = document.getElementById('val-2');

    if (mode === 'compress') {
        label1.textContent = 'Original Size';
        val1.textContent = formatBytes(data.originalSize || 0);

        label2.textContent = 'Compressed Size';
        val2.textContent = formatBytes(data.compressedSize || 0);
    } else {
        // Decompress Mode: Show Input (Compressed) first, then Output (Restored)
        label1.textContent = 'Compressed Size';
        val1.textContent = formatBytes(data.compressedSize || 0);

        label2.textContent = 'Restored Size';
        val2.textContent = formatBytes(data.originalSize || 0);
    }

    if (data.ratio) {
        document.getElementById('stat-ratio').textContent = data.ratio.toFixed(2) + 'x';
    } else {
        document.getElementById('stat-ratio').textContent = '-';
    }

    if (data.entropy) {
        document.getElementById('stat-entropy').textContent = data.entropy.toFixed(2);
    } else {
        document.getElementById('stat-entropy').textContent = '-';
    }

    const downloadLink = document.getElementById('download-link');
    // Use relative path for download
    downloadLink.href = `${BASE_URL}${data.downloadUrl}`;

}

function formatBytes(bytes, decimals = 2) {
    if (bytes === 0) return '0 B';
    const k = 1024;
    const dm = decimals < 0 ? 0 : decimals;
    const sizes = ['B', 'KB', 'MB', 'GB', 'TB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return parseFloat((bytes / Math.pow(k, i)).toFixed(dm)) + ' ' + sizes[i];
}
