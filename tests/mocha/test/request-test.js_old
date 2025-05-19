import request from 'supertest';
import assert from 'assert';

describe('Pico HTTP HttpRequest End-to-End Tests', function () {
  this.timeout(10000); // In case responses are slow

  afterEach(function (done) {
    setTimeout(done, 100); // adjust as needed
  });

  const server = request('http://192.168.50.20');

  it('GET /index.html should return 200 OK with expected welcome text', async () => {
    const res = await server.get('/index.html');
    assert.strictEqual(res.statusCode, 200);
    assert.ok(res.text.includes('Welcome from Ian Archbell') || res.text.includes('<html'), 'Expected index.html content');
  });

  it('POST /api/v1/upload should be handled without crashing (no-op)', async () => {
    const res = await server
      .post('/api/v1/upload')
      .set('Content-Type', 'multipart/form-data')
      .attach('file', Buffer.from('dummy content'), 'dummy.txt');

    // Since your route is a no-op, a 200 or 204 would be valid
    assert.ok(res.statusCode === 200 || res.statusCode === 204);
  });

  it('GET /auth without token should return 401 Unauthorized', async () => {
    const res = await server.get('/auth');
    assert.strictEqual(res.statusCode, 401);
  });

  it('GET /auth with valid token should return authenticated response', async () => {
    const res = await server
      .get('/auth')
      .set('Authorization', 'Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJleHAiOjM2MDAsImlhdCI6MCwibmFtZSI6IkpvaG4gRG9lIiwic3ViIjoiYWRtaW4ifQ.rrJCcGVKN8qckNWNVC2FECqTEECwByEny7F-3mdD88k'); // Assuming `authMiddleware` accepts "testtoken"
    assert.strictEqual(res.statusCode, 200);
    assert.ok(res.text.includes('Authenticated'));
  });

  it('GET /nonexistent should fall back to static file handling (catch-all)', async () => {
    const res = await server.get('/nonexistent');
    assert.ok(res.statusCode === 200 || res.statusCode === 404); // Allow 404 if file not present
  });
});
