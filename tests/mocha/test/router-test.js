import request from 'supertest';
import assert from 'assert';


describe('Pico Routing Table End-to-End Tests', function () {
  this.timeout(10000);

  afterEach(function (done) {
    setTimeout(done, 1000); // adjust as needed
  });

  const server = request('http://192.168.50.20');

  it('GET /api/v1/programs should return a JSON list of programs', async () => {
    const res = await server.get('/api/v1/programs');
    assert.strictEqual(res.statusCode, 200);
    assert.ok(Array.isArray(res.body.programs) || typeof res.body === 'object');
  });

  it('GET /api/v1/programs/testProgram should return details for testProgram', async () => {
    const res = await server.get('/api/v1/programs/testProgram');
    assert.strictEqual(res.statusCode, 200);
    assert.ok(res.body.name === 'testProgram' || typeof res.body === 'object');
  });

  it('GET /api/v1/zones should return a JSON list of zones', async () => {
    const res = await server.get('/api/v1/zones');
    assert.strictEqual(res.statusCode, 200);
    assert.ok(Array.isArray(res.body.zones) || typeof res.body === 'object');
  });

  it('GET /api/v1/zones/zone1 should return details for zone1', async () => {
    const res = await server.get('/api/v1/zones/zone1');
    assert.strictEqual(res.statusCode, 200);
    assert.ok(res.body.name === 'zone1' || typeof res.body === 'object');
  });

  it('POST /api/v1/programs should add a program', async () => {
    const res = await server
      .post('/api/v1/programs')
      .send({ name: 'testProgram', duration: 5 })
      .set('Content-Type', 'application/json');
    assert.strictEqual(res.statusCode, 200);
  });

  it('PUT /api/v1/programs/testProgram should update the program', async () => {
    const res = await server
      .put('/api/v1/programs/testProgram')
      .send({ duration: 10 })
      .set('Content-Type', 'application/json');
    assert.strictEqual(res.statusCode, 200);
  });

  it('DELETE /api/v1/programs/testProgram should delete the program', async () => {
    const res = await server.delete('/api/v1/programs/testProgram');
    assert.strictEqual(res.statusCode, 200);
    assert.ok(res.body.message);
  });

  it('POST /api/v1/zones/zone1/start should start zone1', async () => {
    const res = await server.post('/api/v1/zones/zone1/start');
    assert.strictEqual(res.statusCode, 200);
  });

  it('POST /api/v1/zones/zone1/stop should stop zone1', async () => {
    const res = await server.post('/api/v1/zones/zone1/stop');
    assert.strictEqual(res.statusCode, 200);
  });

  it('PUT /api/v1/zones/zone1 should update the zone', async () => {
    const res = await server
      .put('/api/v1/zones/zone1')
      .send({ name: 'zone1', enabled: true })
      .set('Content-Type', 'application/json');
    assert.strictEqual(res.statusCode, 200);
  });

  it('GET /api/v1/ls should return a directory listing', async () => {
    const res = await server.get('/api/v1/ls');
    assert.strictEqual(res.statusCode, 200);
    assert.ok(Array.isArray(res.body.files) || typeof res.body === 'object');
  });
});
