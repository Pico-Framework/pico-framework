// test/test-app.test.js
import request from 'supertest';
import assert from 'assert';

const BASE_URL = 'http://pico-framework';

describe('Test App HTTP Routes', function () {
  this.timeout(5000);

  it('GET / should return dashboard HTML or view', async () => {
    const res = await request(BASE_URL).get('/');
    assert.strictEqual(res.status, 200);
    assert.match(res.text, /<!DOCTYPE html>|Welcome to PicoFramework!/);
  });

  it('GET /hello should return welcome string', async () => {
    const res = await request(BASE_URL).get('/hello');
    assert.strictEqual(res.status, 200);
    assert.match(res.text, /Welcome to PicoFramework/);
  });

  it('GET /api/v1/temperature should return temperature data', async () => {
    const res = await request(BASE_URL).get('/api/v1/temperature');
    assert.strictEqual(res.status, 200);
    assert.ok('temperature' in res.body);
  });

  it('GET /api/v1/led should return LED state', async () => {
    const res = await request(BASE_URL).get('/api/v1/led');
    assert.strictEqual(res.status, 200);
    assert.ok('state' in res.body);
  });

  it('POST /api/v1/led/1 should set LED on', async () => {
    const res = await request(BASE_URL).post('/api/v1/led/1');
    assert.strictEqual(res.status, 200);
    assert.strictEqual(res.body.state, 1);
  });

  it('GET /api/v1/gpio/16 should return GPIO state', async () => {
    const res = await request(BASE_URL).get('/api/v1/gpio/16');
    assert.strictEqual(res.status, 200);
    assert.strictEqual(res.body.pin, 16);
  });

  it('POST /api/v1/gpio/16/1 should set GPIO 16 to high', async () => {
    const res = await request(BASE_URL).post('/api/v1/gpio/16/1');
    assert.strictEqual(res.status, 200);
    assert.strictEqual(res.body.pin, 16);
    assert.strictEqual(res.body.state, 1);
  });

  it('GET /api/v1/gpios?pin=16&pin=17 should return multiple pin states', async () => {
    const res = await request(BASE_URL).get('/api/v1/gpios?pin=16&pin=17');
    assert.strictEqual(res.status, 200);
    assert.ok(Array.isArray(res.body));
    assert.ok(res.body.length > 0);
  });

  it('GET /ls should return directory listing', async () => {
    const res = await request(BASE_URL).get('/ls');
    assert.strictEqual(res.status, 200);
    assert.ok(Array.isArray(res.body));
  });

  it('POST /api/v1/upload should accept file upload', async () => {
    const res = await request(BASE_URL)
      .post('/api/v1/upload')
      .attach('file', Buffer.from('test content'), 'test.txt');
    assert.strictEqual(res.status, 200);
    assert.strictEqual(res.body.success, true);
  });

  it('DELETE /uploads/test.txt should delete uploaded file', async () => {
    const res = await request(BASE_URL).delete('/uploads/test.txt');
    // This may return 200 or 404 depending on file presence
    assert.ok([200, 404].includes(res.status));
  });

  it('GET /unknown.txt should return fallback static or 404', async () => {
    const res = await request(BASE_URL).get('/unknown.txt');
    assert.ok([200, 404].includes(res.status));
  });
});
