// test/weather.test.js
import request from 'supertest';
import assert from 'assert';

const BASE_URL = 'http://Pico-Framework';

describe('Weather App API Tests', function () {
  this.timeout(5000);

  it('GET / should return the weather HTML page or content', async () => {
    const res = await request(BASE_URL).get('/');
    assert.strictEqual(res.status, 200);
    assert.ok(res.text.includes('<') || res.text.includes('weather'), 'Expected HTML content');
  });

  it('GET /api/v1/weather should return weather JSON with current and forecast', async () => {
    const res = await request(BASE_URL).get('/api/v1/weather');
    assert.strictEqual(res.status, 200);
    assert.ok(res.body);
    assert.ok('location' in res.body);
    assert.ok('current' in res.body);
    assert.ok('forecast' in res.body);

    assert.ok(typeof res.body.current.temperature === 'number');
    assert.ok(Array.isArray(res.body.forecast));
  });

  it('GET /api/v1/weather?lat=37.7749&lon=-122.4194 should override location', async () => {
    const res = await request(BASE_URL).get('/api/v1/weather?lat=37.7749&lon=-122.4194');
    assert.strictEqual(res.status, 200);
    assert.ok(res.body.location);
    assert.ok(res.body.current.temperature !== undefined);
  });
});
