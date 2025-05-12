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

let lastRoute = null;

function loadRoute() {
  const fullHash = location.hash || '#/';
  const [hashPath] = fullHash.split('?');  // Strip query string
  updateActiveNav(hashPath);

  if (hashPath === lastRoute) return;
  lastRoute = hashPath;

  const route = routes[hashPath] || 'sprinkler-dashboard';

  if (!customElements.get(route)) {
    console.warn(`Unknown route component: ${route}`);
    return;
  }

  const main = document.getElementById('app');
  main.innerHTML = `<${route}></${route}>`;
}



window.addEventListener('hashchange', loadRoute);
window.addEventListener('DOMContentLoaded', loadRoute);

function updateActiveNav(route) {
  document.querySelectorAll('nav button').forEach(btn => {
    btn.classList.remove('active');
    const target = btn.dataset.route;
    if (target === route) btn.classList.add('active');
  });
}
