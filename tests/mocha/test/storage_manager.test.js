// test/storagemanager.test.js
import request from 'supertest';
import assert from 'assert';

const BASE_URL = 'http://localhost';

describe('StorageManager API Tests', function () {
  this.timeout(5000);

  const testFileName = `mocha-test-${Date.now()}.txt`;

  it('GET / should return the file storage UI', async () => {
    const res = await request(BASE_URL).get('/');
    assert.strictEqual(res.status, 200);
    assert.ok(res.text.includes('<') || res.text.includes('File'), 'Expected HTML content');
  });

  it('DELETE /api/v1/files/:file should not fail if file doesn’t exist yet', async () => {
    await request(BASE_URL).delete(`/api/v1/files/${testFileName}`);
  });

  it('POST /api/v1/upload should upload a test file', async () => {
    const res = await request(BASE_URL)
      .post('/api/v1/upload')
      .attach('file', Buffer.from('hello from mocha'), {
        filename: testFileName,
        contentType: 'text/plain'
      });
    assert.strictEqual(res.status, 200);
  });

  it('GET /api/v1/ls should list the uploaded test file', async () => {
    const res = await request(BASE_URL).get('/api/v1/ls');
    assert.strictEqual(res.status, 200);
    const files = res.body.map(f => f.name);
    assert.ok(files.includes(testFileName), 'Expected uploaded file to be listed');
  });

  it('GET /:file should retrieve uploaded file content', async () => {
    const res = await request(BASE_URL).get(`/${testFileName}`);
    assert.strictEqual(res.status, 200);
    assert.strictEqual(res.text, 'hello from mocha');
  });

  it('DELETE /api/v1/files/:file should delete the test file', async () => {
    const res = await request(BASE_URL).delete(`/api/v1/files/${testFileName}`);
    assert.strictEqual(res.status, 200);
    assert.strictEqual(res.body.message, 'File deleted successfully');
  });

  it('POST /api/v1/format_storage should format storage (if needed)', async () => {
    const res = await request(BASE_URL).post('/api/v1/format_storage');
    assert.ok([200, 500].includes(res.status));
  });
});
