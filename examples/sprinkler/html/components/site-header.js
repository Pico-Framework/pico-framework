class SiteHeader extends HTMLElement {
  connectedCallback() {
    this.innerHTML = `
      <nav class="site-header">
        <button data-route="#/">Home</button>
        <button data-route="#/zones">Zones</button>
        <button data-route="#/programs">Programs</button>
        <button data-route="#/log">Log</button>
      </nav>
    `;
  }
}
customElements.define('site-header', SiteHeader);

document.querySelectorAll('nav button').forEach(btn => {
  btn.addEventListener('click', e => {
    const target = btn.dataset.route;
    if (target === location.hash) {
      e.preventDefault();
      console.log('Ignoring redundant route click');
    } else {
      location.hash = target;
    }
  });
});
  
  