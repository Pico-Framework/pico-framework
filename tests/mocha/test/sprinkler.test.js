import request from 'supertest';
import assert from 'assert';

const BASE_URL = 'http://pico-framework';

describe('Sprinkler App API Tests', function () {
  this.timeout(5000);

  //
  // ZONE ROUTES
  //
  it('GET /api/v1/zones should return all zones', async () => {
    const res = await request(BASE_URL).get('/api/v1/zones');
    assert.strictEqual(res.status, 200);
    assert.ok(Array.isArray(res.body));
  });

  it('GET /api/v1/zones/Front%20Lawn should return zone details', async () => {
    const res = await request(BASE_URL).get('/api/v1/zones/Front%20Lawn');
    assert.strictEqual(res.status, 200);
    assert.ok(res.body.name === 'Front Lawn');
  });

  it('PUT /api/v1/zones/zone1 should update a zone', async () => {
    const res = await request(BASE_URL)
      .put('/api/v1/zones/1')
      .send({
        id: '1',
        name: 'Front Lawn',
        gpioPin: 1,
        active: true,
        image: 'Front Lawn.jpg'
      });
    assert.strictEqual(res.status, 200);
    assert.strictEqual(res.body.success, true);
  });

  it('POST /api/v1/zones/Front%20Lawn/start should start a zone', async () => {
    const res = await request(BASE_URL).post('/api/v1/zones/Front%20Lawn/start');
    assert.strictEqual(res.status, 200);
    assert.strictEqual(res.body.success, true);
  });

  it('POST /api/v1/zones/Front%20Lawn/stop should stop a zone', async () => {
    const res = await request(BASE_URL).post('/api/v1/zones/Front%20Lawn/stop');
    assert.strictEqual(res.status, 200);
    assert.strictEqual(res.body.success, true);
  });

  it('GET /api/v1/image_exists/Front%20Lawn.jpg should return exists status', async () => {
    const res = await request(BASE_URL).get('/api/v1/image_exists/Front%20Lawn.jpg');
    assert.strictEqual(res.status, 200);
    assert.strictEqual(res.body.success, true);
    assert.ok(typeof res.body.exists === 'boolean');
  });

  //
  // PROGRAM ROUTES
  //
  it('GET /api/v1/programs should list all programs', async () => {
    const res = await request(BASE_URL).get('/api/v1/programs');
    assert.strictEqual(res.status, 200);
    assert.ok(Array.isArray(res.body));
  });

  it('POST /api/v1/programs should create a program', async () => {
    const res = await request(BASE_URL)
      .post('/api/v1/programs')
      .send({
        name: 'Evening Watering',
        start: '19:00',
        days: 73, // Mon, Wed, Fri
        zones: [
          { zone: 'Front Lawn', duration: 120 },
          { zone: 'Back Garden', duration: 180 }
        ]
      });
    assert.strictEqual(res.status, 200);
    assert.strictEqual(res.body.success, true);
  });

  it('GET /api/v1/programs/Evening%20Watering should fetch a program', async () => {
    const res = await request(BASE_URL).get('/api/v1/programs/Evening%20Watering');
    assert.strictEqual(res.status, 200);
    assert.strictEqual(res.body.name, 'Evening Watering');
  });

  it('PUT /api/v1/programs/Evening%20Watering should update a program', async () => {
    const res = await request(BASE_URL)
      .put('/api/v1/programs/Evening%20Watering')
      .send({
        start: '20:00',
        days: 73,
        zones: [{ zone: 'Front Lawn', duration: 240 }]
      });
    assert.strictEqual(res.status, 200);
    assert.strictEqual(res.body.success, true);
  });

  it('DELETE /api/v1/programs/Evening%20Watering should delete a program', async () => {
    const res = await request(BASE_URL).delete('/api/v1/programs/Evening%20Watering');
    assert.strictEqual(res.status, 200);
    assert.strictEqual(res.body.success, true);
  });

  it('GET /api/v1/next-schedule should return next program or none', async () => {
    const res = await request(BASE_URL).get('/api/v1/next-schedule');
    assert.strictEqual(res.status, 200);
    assert.ok(['scheduled', 'none'].includes(res.body.status));
  });

  it('POST /api/v1/test-program should create a test run', async () => {
    const res = await request(BASE_URL).post('/api/v1/test-program');
    assert.strictEqual(res.status, 200);
    assert.strictEqual(res.body.success, true);
    assert.ok(res.body.scheduled);
  });

  //
  // LOG AND UPLOAD
  //
  it('POST /api/v1/upload should accept file upload', async () => {
    const uniqueName = `test-upload-${Date.now()}.jpg`;
    const res = await request(BASE_URL)
      .post('/api/v1/upload')
      .attach('file', Buffer.from('fake image'), {
        filename: uniqueName,
        contentType: 'image/jpeg'
      });
    assert.strictEqual(res.status, 200);
    assert.strictEqual(res.body.success, true);
  });

  it('GET /api/v1/logs/summary should return plain log text', async () => {
    const res = await request(BASE_URL).get('/api/v1/logs/summary');
    assert.strictEqual(res.status, 200);
    assert.ok(res.text.includes('[INFO]'));
  });

  it('GET /api/v1/logs/summaryJson should return zone log summary', async () => {
    const res = await request(BASE_URL).get('/api/v1/logs/summaryJson');
    assert.strictEqual(res.status, 200);
    assert.ok('zones' in res.body);
  });
});
