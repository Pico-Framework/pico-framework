class SiteHeader extends HTMLElement {
    connectedCallback() {
      this.innerHTML = `
        <nav class="site-header">
          <a href="#/">Dashboard</a>
          <a href="#/zones">Zones</a>
          <a href="#/programs">Programs</a>
          <a href="#/log">Log</a>
        </nav>
      `;
    }
  }
  customElements.define('site-header', SiteHeader);

  document.querySelectorAll('nav a').forEach(link => {
    link.addEventListener('click', e => {
      if (link.getAttribute('href') === location.hash) {
        e.preventDefault(); // Stop navigation entirely
        console.log('Ignoring redundant route click');
      }
    });
  });
  
  