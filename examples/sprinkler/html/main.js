import './components/site-header.js';
import './components/sprinkler-dashboard.js';
import './components/zone-editor.js';
import './components/program-list.js';
import './components/program-editor.js';

const routes = {
  '#/': 'sprinkler-dashboard',
  '#/zones': 'zone-editor',
  '#/programs': 'program-list',
  '#/programs/edit': 'program-editor'
};

function loadRoute() {
  const route = routes[location.hash] || 'dashboard';
  const main = document.getElementById('app');
  main.innerHTML = `<${route}></${route}>`;
}

window.addEventListener('hashchange', loadRoute);
window.addEventListener('DOMContentLoaded', loadRoute);
