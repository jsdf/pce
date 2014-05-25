module.exports = {
  loadingStatus: loadingStatus,
}

function setElAttrs(el, attrs) {
  Object.keys(attrs).forEach(function(key) {
    el.setAttribute(key, attrs[key])
  })
  return el
}

function emptyEl(el) {
  while (el.firstChild) el.removeChild(el.firstChild)
  return el
}

function loadingStatus(loadingStatusEl) {
  var initialStatus = loadingStatusEl.innerText
  
  emptyEl(loadingStatusEl)

  var statusEl = setElAttrs(document.createElement("div"), {innerHTML: initialStatus})

  var progressEl = setElAttrs(document.createElement("progress"), {
    value: 0,
    max: 100,
    hidden: true,
  })

  progressEl.style.display = 'inline'

  loadingStatusEl.appendChild(statusEl)
  loadingStatusEl.appendChild(progressEl)

  return  {
    totalDependencies: 0,
    update: function(remainingDependencies) {
      this.totalDependencies = Math.max(this.totalDependencies, remainingDependencies)
      this.setStatus(remainingDependencies)    
    },

    setStatus: function (remainingDependencies) {
      if (this.setStatus.interval) clearInterval(this.setStatus.interval)

      var loadedDependiences = this.totalDependencies - remainingDependencies

      if (remainingDependencies) {
        statusEl.innerHTML = 'Loading... ('+loadedDependiences+'/'+this.totalDependencies+')'
        setElAttrs(progressEl, {
          value: loadedDependiences*100,
          max: this.totalDependencies*100,
          hidden: false,
        })
      } else {
        // close progress element
        setElAttrs(progressEl, {
          value: 0,
          max: 0,
          hidden: true,
        })
        loadingStatusEl.style.display = 'none'
      }
    },
  }
}

