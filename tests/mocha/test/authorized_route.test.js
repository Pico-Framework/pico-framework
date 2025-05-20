import request from 'supertest';
import assert from 'assert';

const baseUrl = 'http://Pico-Framework';

let token = null;

describe('hello_auth Authentication Demo Tests', function () {
    this.timeout(10000); // Increase timeout in case of slow device

    const timestamp = Date.now();
    const testUser = {
        username: `testuser_${timestamp}`,
        password: 'testpass'
    };

    it('GET / should redirect to /login', async function () {
        const res = await request(baseUrl).get('/');
        assert.strictEqual(res.statusCode, 302);
        assert.strictEqual(res.headers.location, '/login');
    });

    it('GET /login should return login page', async function () {
        const res = await request(baseUrl).get('/login');
        assert.strictEqual(res.statusCode, 200);
        assert.match(res.text, /<!DOCTYPE/i);
    });

    it('POST /signup should create a new user', async function () {
        const res = await request(baseUrl)
            .post('/signup')
            .set('Content-Type', 'application/json')
            .send(JSON.stringify(testUser));
        assert.strictEqual(res.statusCode, 200);
    });

    it('POST /auth should authenticate and return JWT token', async function () {
        const res = await request(baseUrl)
            .post('/auth')
            .send(testUser)
            .set('Content-Type', 'application/json');
        assert.strictEqual(res.statusCode, 200);
        assert.ok(res.body.token);
        token = res.body.token;     
    });

    it('GET /api/v1/protected-data without token should fail', async function () {
        const res = await request(baseUrl).get('/api/v1/protected-data');
        assert.strictEqual(res.statusCode, 401);
    });

    it('GET /api/v1/protected-data with valid token should succeed', async function () {
        console.log(token);
        const res = await request(baseUrl)
            .get('/api/v1/protected-data')
            .set('Authorization', `Bearer ${token}`);
        assert.strictEqual(res.statusCode, 200);
    });
});
