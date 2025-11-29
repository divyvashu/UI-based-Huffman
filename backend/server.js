const express = require('express');
const multer = require('multer');
const cors = require('cors');
const { execFile } = require('child_process');
const path = require('path');
const fs = require('fs');

const app = express();
const PORT = process.env.PORT || 3000;

// Enable CORS
app.use(cors());
app.use(express.json());

// Serve Frontend Static Files
app.use(express.static(path.join(__dirname, '../../frontend')));

// Ensure uploads directory exists
const uploadDir = path.join(__dirname, 'uploads');
if (!fs.existsSync(uploadDir)) {
    fs.mkdirSync(uploadDir);
}

// Configure Multer
const storage = multer.diskStorage({
    destination: (req, file, cb) => {
        cb(null, uploadDir);
    },
    filename: (req, file, cb) => {
        cb(null, Date.now() + '-' + file.originalname);
    }
});

const upload = multer({ storage: storage });

// Path to C executable
const EXE_PATH = path.resolve(__dirname, '../c/huffman');

// Helper to run C program
const runHuffman = (command, inputFile, outputFile) => {
    return new Promise((resolve, reject) => {
        execFile(EXE_PATH, [command, inputFile, outputFile], (error, stdout, stderr) => {
            if (error) {
                console.error('Exec error:', error);
                return reject(error);
            }
            try {
                // Parse the JSON output from stdout
                // stdout might contain other text if not careful, but our C program prints only JSON on success
                // We trim whitespace to be safe
                const stats = JSON.parse(stdout.trim());
                resolve(stats);
            } catch (e) {
                console.error('JSON Parse error:', e, 'Stdout:', stdout);
                reject(new Error('Failed to parse backend output'));
            }
        });
    });
};

// API Routes

app.post('/api/compress', upload.single('file'), async (req, res) => {
    if (!req.file) {
        return res.status(400).json({ error: 'No file uploaded' });
    }

    const inputFile = req.file.path;
    const outputFileName = req.file.filename + '.huf';
    const outputFile = path.join(uploadDir, outputFileName);

    // Generate a nice name for download: original.ext -> original.huf
    const originalName = req.file.originalname;
    const downloadName = originalName.split('.').slice(0, -1).join('.') + '.huf';

    try {
        const stats = await runHuffman('compress', inputFile, outputFile);

        res.json({
            ...stats,
            downloadUrl: `/api/download/${outputFileName}?name=${encodeURIComponent(downloadName)}`
        });
    } catch (err) {
        res.status(500).json({ error: 'Compression failed', details: err.message });
    }
});

app.post('/api/decompress', upload.single('file'), async (req, res) => {
    if (!req.file) {
        return res.status(400).json({ error: 'No file uploaded' });
    }

    const inputFile = req.file.path;
    const outputFileName = req.file.filename + '.decoded';
    const outputFile = path.join(uploadDir, outputFileName);

    // Generate a nice name for download: file.huf -> file_decoded.txt (or just remove .huf)
    const originalName = req.file.originalname;
    let downloadName = originalName.replace('.huf', '');
    if (downloadName === originalName) {
        downloadName += '_decoded';
    }
    // If it didn't have an extension, maybe add .txt default? 
    // For now, let's just ensure it's not empty.
    if (!downloadName) downloadName = 'decoded_file';

    try {
        const stats = await runHuffman('decompress', inputFile, outputFile);

        res.json({
            ...stats,
            downloadUrl: `/api/download/${outputFileName}?name=${encodeURIComponent(downloadName)}`
        });
    } catch (err) {
        res.status(500).json({ error: 'Decompression failed', details: err.message });
    }
});

app.get('/api/download/:filename', (req, res) => {
    const filename = req.params.filename;
    const filePath = path.join(uploadDir, filename);
    const downloadName = req.query.name || filename;

    if (fs.existsSync(filePath)) {
        res.download(filePath, downloadName);
    } else {
        res.status(404).json({ error: 'File not found' });
    }
});

app.listen(PORT, () => {
    console.log(`Server running on port ${PORT}`);
});
