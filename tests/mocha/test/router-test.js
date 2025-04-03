import request from 'supertest';
import { expect } from 'chai';

// Replace with the actual IP address or hostname of your Pico device.
const picoUrl = 'http:192.168.50.20';

describe('Pico Routing Table End-to-End Tests', function() {
  this.timeout(5000);

  it('GET /index.html should return static content from the file server', function(done) {
    request(picoUrl)
      .get('/index.html')
      .expect(200)
      .end((err, res) => {
         if (err) return done(err);
         // Assuming your file server returns HTML with a title or known keyword.
         expect(res.text).to.include('<!DOCTYPE html>');
         done();
      });
  });

  it('GET /api/v1/programs should return a JSON list of programs', function(done) {
    request(picoUrl)
      .get('/api/v1/programs')
      .expect(200)
      .end((err, res) => {
         if (err) return done(err);
         // Assume scheduler returns an array even if empty.
         expect(res.body).to.be.an('array');
         done();
      });
  });

  it('GET /api/v1/programs/testProgram should return details for testProgram', function(done) {
    request(picoUrl)
      .get('/api/v1/programs/testProgram')
      .expect(200)
      .end((err, res) => {
         if (err) return done(err);
         // For testing, assume the response JSON includes a "name" field.
         expect(res.body).to.have.property('name', 'testProgram');
         done();
      });
  });

  it('GET /api/v1/zones should return a JSON list of zones', function(done) {
    request(picoUrl)
      .get('/api/v1/zones')
      .expect(200)
      .end((err, res) => {
         if (err) return done(err);
         expect(res.body).to.be.an('array');
         done();
      });
  });

  it('GET /api/v1/zones/zone1 should return details for zone1', function(done) {
    request(picoUrl)
      .get('/api/v1/zones/zone1')
      .expect(200)
      .end((err, res) => {
         if (err) return done(err);
         expect(res.body).to.have.property('name', 'zone1');
         done();
      });
  });

  it('POST /api/v1/programs should add a program and return a confirmation', function(done) {
    // For testing, we send JSON data. Ensure your handler accepts JSON.
    request(picoUrl)
      .post('/api/v1/programs')
      .send({ name: 'newProgram', config: {} })
      .set('Content-Type', 'application/json')
      .expect(200)
      .end((err, res) => {
         if (err) return done(err);
         // Assume the scheduler handler returns a confirmation message.
         expect(res.body).to.have.property('message').that.includes('added');
         done();
      });
  });

  it('POST /api/v1/zones/zone1/start should start zone1', function(done) {
    request(picoUrl)
      .post('/api/v1/zones/zone1/start')
      .expect(200)
      .end((err, res) => {
         if (err) return done(err);
         // Expect a message confirming that zone1 was started.
         expect(res.body).to.have.property('message').that.includes('started');
         done();
      });
  });

  it('POST /api/v1/zones/zone1/stop should stop zone1', function(done) {
    request(picoUrl)
      .post('/api/v1/zones/zone1/stop')
      .expect(200)
      .end((err, res) => {
         if (err) return done(err);
         // Expect a message confirming that zone1 was stopped.
         expect(res.body).to.have.property('message').that.includes('stopped');
         done();
      });
  });

  it('PUT /api/v1/programs/testProgram should update program details', function(done) {
    request(picoUrl)
      .put('/api/v1/programs/testProgram')
      .send({ config: { updated: true } })
      .set('Content-Type', 'application/json')
      .expect(200)
      .end((err, res) => {
         if (err) return done(err);
         expect(res.body).to.have.property('message').that.includes('updated');
         done();
      });
  });

  it('PUT /api/v1/zones/zone1 should update zone details', function(done) {
    request(picoUrl)
      .put('/api/v1/zones/zone1')
      .send({ config: { brightness: 75 } })
      .set('Content-Type', 'application/json')
      .expect(200)
      .end((err, res) => {
         if (err) return done(err);
         expect(res.body).to.have.property('message').that.includes('updated');
         done();
      });
  });

  it('DELETE /api/v1/programs/testProgram should delete the program', function(done) {
    request(picoUrl)
      .delete('/api/v1/programs/testProgram')
      .expect(200)
      .end((err, res) => {
         if (err) return done(err);
         expect(res.body).to.have.property('message').that.includes('deleted');
         done();
      });
  });

  it('GET /api/v1/ls should return a directory listing', function(done) {
    request(picoUrl)
      .get('/api/v1/ls')
      .expect(200)
      .end((err, res) => {
         if (err) return done(err);
         // Assume a JSON array listing files/directories is returned.
         expect(res.body).to.be.an('array');
         done();
      });
  });

  it('GET /auth should return an authenticated response', function(done) {
    request(picoUrl)
      .get('/auth')
      .expect(200)
      .end((err, res) => {
         if (err) return done(err);
         // This route is defined inline to return a JSON string.
         expect(res.text).to.equal('{"message":"Authenticated request"}');
         done();
      });
  });

  it('GET /nonexistent should fall back to static file handling (catch-all route)', function(done) {
    request(picoUrl)
      .get('/nonexistent')
      .expect(200)
      .end((err, res) => {
         if (err) return done(err);
         // The catch-all route should return static content (could be a 404 page or custom message).
         expect(res.text).to.not.be.empty;
         done();
      });
  });
});
