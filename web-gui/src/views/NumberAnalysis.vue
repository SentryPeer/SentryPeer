<template>
  <div class="row">
    <div class="col-auto">
      <div class="card mb-5">
        <div class="card-header">Time Frame</div>
        <div class="card-body">
          <nav class="nav nav-pills flex-column">
            <a class="nav-link active" aria-current="page" href="#">1 Hour</a>
            <a class="nav-link" href="#">24 Hours</a>
            <a class="nav-link" href="#">7 Days</a>
            <a class="nav-link" href="#">30 Days</a>
          </nav>
        </div>
      </div>
    </div>
    <div class="col">
      <div class="card mb-5">
        <div class="card-header">
          Data collected between 17:00 and 18:00 UTC
        </div>
        <div class="card-body">
          <table class="table">
            <thead>
              <tr>
                <th scope="col">Numbers attempted</th>
                <th class="text-end" scope="col">Count</th>
              </tr>
            </thead>
            <tbody>
              <tr>
                <td>+441234567899</td>
                <td class="text-end">1576481</td>
              </tr>
            </tbody>
          </table>
        </div>
      </div>
    </div>
  </div>
</template>
